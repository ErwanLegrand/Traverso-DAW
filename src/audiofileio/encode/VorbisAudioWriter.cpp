/*
Copyright (C) 2007 Ben Levitt 
 * Based on the ogg encoder module for K3b.
 * Copyright (C) 1998-2007 Sebastian Trueg <trueg@k3b.org>

This file is part of Traverso

Traverso is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.

*/

#include "VorbisAudioWriter.h"

#include <stdio.h>
#include <vorbis/vorbisenc.h>

// for the random generator
#include <stdlib.h>
#include <time.h>

#include <QString>

RELAYTOOL_VORBISFILE;

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"


class VorbisAudioWriter::Private
{
public:
	Private()
		: manualBitrate(false),
		qualityLevel(4),
		bitrateUpper(-1),
		bitrateNominal(-1),
		bitrateLower(-1),
		sampleRate(0),
		oggStream(0),
		oggPage(0),
		oggPacket(0),
		vorbisInfo(0),
		vorbisComment(0),
		vorbisDspState(0),
		vorbisBlock(0),
		fid(0),
		headersWritten(false) {
	}
	
	// encoding settings
	bool manualBitrate;
	// 0 to 10 -> 0.0 - 1.0
	int qualityLevel;
	int bitrateUpper;
	int bitrateNominal;
	int bitrateLower;
	int sampleRate;
	
	// encoding structures
	ogg_stream_state *oggStream;       // take physical pages, weld into a logical stream of packets
	ogg_page         *oggPage;         // one Ogg bitstream page.  Vorbis packets are inside
	ogg_packet       *oggPacket;       // one raw packet of data for decode
	vorbis_info      *vorbisInfo;      // struct that stores all the static vorbis bitstream settings
	vorbis_comment   *vorbisComment;   // struct that stores all the user comments
	vorbis_dsp_state *vorbisDspState;  // central working state for the packet->PCM decoder
	vorbis_block     *vorbisBlock;     // local working space for packet->PCM decode
	
	FILE *fid;
	bool headersWritten;
};


VorbisAudioWriter::VorbisAudioWriter()
 : AbstractAudioWriter()
{
	d = new Private();
}


VorbisAudioWriter::~VorbisAudioWriter()
{
  cleanup();
  delete d;
}


const char* VorbisAudioWriter::get_extension()
{
	return ".ogg";
}


bool VorbisAudioWriter::set_format_attribute(const QString& key, const QString& value)
{
	if (key == "mode") {
		d->manualBitrate = (value == "manual");
		return true;
	}
	else if (key == "bitrateLower") {
		d->bitrateLower = value.toInt();
		return true;
	}
	else if (key == "bitrateNominal") {
		d->bitrateNominal = value.toInt();
		return true;
	}
	else if (key == "bitrateUpper") {
		d->bitrateUpper = value.toInt();
		return true;
	}
	else if (key == "vbrQuality") { // -1 to 10
		d->qualityLevel = value.toInt();
		return true;
	}
	
	return false;
}


bool VorbisAudioWriter::open_private()
{
	cleanup();
	
	d->fid = fopen(m_fileName.toUtf8().data(), "w+");
	if (!d->fid) {
		return false;
	}
	
	d->oggPage = new ogg_page;
	d->oggPacket = new ogg_packet;
	d->vorbisInfo = new vorbis_info;
	
	vorbis_info_init(d->vorbisInfo);
	
	int ret = 0;
	
	if (d->manualBitrate) {
		ret = vorbis_encode_init(d->vorbisInfo, 
					 m_channels,
					 m_rate,
					 d->bitrateUpper != -1 ? d->bitrateUpper*1000 : -1,
					 d->bitrateNominal != -1 ? d->bitrateNominal*1000 : -1,
					 d->bitrateLower != -1 ? d->bitrateLower*1000 : -1 );
	}
	else {
		if (d->qualityLevel < -1) {
			d->qualityLevel = -1;
		}
		else if (d->qualityLevel > 10) {
			d->qualityLevel = 10;
		}
		
		ret = vorbis_encode_init_vbr(d->vorbisInfo, 
						m_channels,
						m_rate,
						(float)d->qualityLevel/10.0);
	}
	
	if (ret) {
		PERROR("vorbis_encode_init failed: %d", ret);
		cleanup();
		return false;
	}
	
	// init the comment stuff
	d->vorbisComment = new vorbis_comment;
	vorbis_comment_init(d->vorbisComment);
	
	// add the encoder tag (so everybody knows we did it! ;)
	vorbis_comment_add_tag(d->vorbisComment, (char*)"ENCODER", (char*)"Traverso");
	
	// set up the analysis state and auxiliary encoding storage
	d->vorbisDspState = new vorbis_dsp_state;
	d->vorbisBlock = new vorbis_block;
	vorbis_analysis_init(d->vorbisDspState, d->vorbisInfo);
	vorbis_block_init(d->vorbisDspState, d->vorbisBlock);
	
	// set up our packet->stream encoder
	// pick a random serial number; that way we can more likely build
	// chained streams just by concatenation
	d->oggStream = new ogg_stream_state;
	srand(time(0));
	ogg_stream_init(d->oggStream, rand());
	
	return true;
}


bool VorbisAudioWriter::writeOggHeaders()
{
	if (!d->oggStream) {
		PERROR("call to writeOggHeaders without init.");
		return false;
	}
	if (d->headersWritten) {
		PERROR("headers already written.");
		return true;
	}
	
	//
	// Vorbis streams begin with three headers; the initial header (with
	// most of the codec setup parameters) which is mandated by the Ogg
	// bitstream spec.  The second header holds any comment fields.  The
	// third header holds the bitstream codebook.  We merely need to
	// make the headers, then pass them to libvorbis one at a time;
	// libvorbis handles the additional Ogg bitstream constraints
	//
	ogg_packet header;
	ogg_packet header_comm;
	ogg_packet header_code;
	
	vorbis_analysis_headerout(d->vorbisDspState,
				d->vorbisComment,
				&header,
				&header_comm,
				&header_code);
	
	// automatically placed in its own page
	ogg_stream_packetin(d->oggStream, &header);
	ogg_stream_packetin(d->oggStream, &header_comm);
	ogg_stream_packetin(d->oggStream, &header_code);
	
	//
	// This ensures the actual
	// audio data will start on a new page, as per spec
	//
	QByteArray data;
	while (ogg_stream_flush(d->oggStream, d->oggPage)) {
		fwrite((char*)d->oggPage->header, 1, d->oggPage->header_len, d->fid);
		fwrite((char*)d->oggPage->body, 1, d->oggPage->body_len, d->fid);
	}
	
	d->headersWritten = true;
	
	return true;
}


nframes_t VorbisAudioWriter::write_private(void* buffer, nframes_t frameCount)
{
	if (!d->headersWritten) {
		if (!writeOggHeaders()) {
			return 0;
		}
	}
	
	char* data = (char*)buffer;
	
	// expose the buffer to submit data
	float** writeBuffer = vorbis_analysis_buffer(d->vorbisDspState, frameCount);
	
	if (!writeBuffer) {
		return 0;
	}
	
	// uninterleave samples
	if (m_channels == 1) {
		for (nframes_t i = 0; i < frameCount; i++) {
			// Currently assumes 16bit audio
			writeBuffer[0][i]=( (data[i*2+1]<<8) | (0x00ff&(int)data[i*2]) ) / 32768.f;
		}
	}
	else {
		for (nframes_t i = 0; i < frameCount; i++) {
			// Currently assumes 16bit audio
			writeBuffer[0][i]=( (data[i*4+1]<<8) | (0x00ff&(int)data[i*4]) ) / 32768.f;
			writeBuffer[1][i]=( (data[i*4+3]<<8) | (0x00ff&(int)data[i*4+2]) ) / 32768.f;
		}
	}
	
	// tell the library how much we actually submitted
	vorbis_analysis_wrote(d->vorbisDspState, frameCount);
	
	return ((flushVorbis() != -1) ? frameCount : 0);
}


long VorbisAudioWriter::flushVorbis()
{
	// vorbis does some data preanalysis, then divvies up blocks for
	// more involved (potentially parallel) processing.  Get a single
	// block for encoding now
	long writtenData = 0;
	bool success = true;
	
	while (vorbis_analysis_blockout( d->vorbisDspState, d->vorbisBlock ) == 1) {
		// analysis
		vorbis_analysis(d->vorbisBlock, 0);
		vorbis_bitrate_addblock(d->vorbisBlock);
		
		while (vorbis_bitrate_flushpacket(d->vorbisDspState, d->oggPacket)) {
			// weld the packet into the bitstream
			ogg_stream_packetin(d->oggStream, d->oggPacket);
			
			// write out pages (if any)
			while (ogg_stream_pageout(d->oggStream, d->oggPage)) {
				if (fwrite((char*)d->oggPage->header, 1, d->oggPage->header_len, d->fid) != (uint)d->oggPage->header_len
				|| fwrite((char*)d->oggPage->body, 1, d->oggPage->body_len, d->fid) != (uint)d->oggPage->body_len) {
					success = false;
				}
				writtenData += (d->oggPage->header_len + d->oggPage->body_len);
			}
		}
	}
	
	return ((success) ? writtenData : -1);
}


void VorbisAudioWriter::cleanup()
{
	if (d->oggStream) {
		ogg_stream_clear(d->oggStream);
		delete d->oggStream;
		d->oggStream = 0;
	}
	if (d->vorbisBlock) {
		vorbis_block_clear(d->vorbisBlock);
		delete d->vorbisBlock;
		d->vorbisBlock = 0;
	}
	if (d->vorbisDspState) {
		vorbis_dsp_clear(d->vorbisDspState);
		delete d->vorbisDspState;
		d->vorbisDspState = 0;
	}
	if (d->vorbisComment) {
		vorbis_comment_clear(d->vorbisComment);
		delete d->vorbisComment;
		d->vorbisComment = 0;
	}
	if (d->vorbisInfo) {
		vorbis_info_clear(d->vorbisInfo);
		delete d->vorbisInfo;
		d->vorbisInfo = 0;
	}
	
	// ogg_page and ogg_packet structs always point to storage in
	// libvorbis.  They're never freed or manipulated directly
	if (d->oggPage) {
		delete d->oggPage;
		d->oggPage = 0;
	}
	if (d->oggPacket) {
		delete d->oggPacket;
		d->oggPacket = 0;
	}
	if (d->fid) {
		fclose(d->fid);
		d->fid = 0;
	}
	
	d->headersWritten = false;
}


bool VorbisAudioWriter::close_private()
{
	bool success = true;
	
	if (d->vorbisDspState) {
		vorbis_analysis_wrote(d->vorbisDspState, 0);
		if (flushVorbis() == -1) {
			success = false;
		}
	}
	cleanup();
	
	return success;
}

