/*
Copyright (C) 2007 Ben Levitt 
 * This file is based on the mp3 decoding plugin of the K3b project.
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

#include "MadAudioReader.h"
#include <QFile>
#include <QString>
#include <QVector>

RELAYTOOL_MAD

// Always put me below _all_ includes, this is needed
// in case we run with memory leak detection enabled!
#include "Debugger.h"




static const int INPUT_BUFFER_SIZE = 5*8192;

class K3bMad
{
public:
	K3bMad();
	~K3bMad();
	
	bool open(const QString& filename);
	
	/**
	 * @return true if the mad stream contains data
	 *         false if there is no data left or an error occurred.
	 *         In the latter case inputError() returns true.
	 */
	bool fillStreamBuffer();
	
	/**
	 * Skip id3 tags.
	 *
	 * This will reset the input file.
	 */
	bool skipTag();
	
	/**
	 * Find first frame and seek to the beginning of that frame.
	 * This is used to skip the junk that many mp3 files start with.
	 */
	bool seekFirstHeader();
	
	bool eof() const;
	bool inputError() const;
	
	/**
	 * Current position in theinput file. This does NOT
	 * care about the status of the mad stream. Use streamPos()
	 * in that case.
	 */
	qint64 inputPos() const;
	
	/**
	 * Current absolut position of the decoder stream.
	 */
	qint64 streamPos() const;
	bool inputSeek(qint64 pos);
	
	void initMad();
	void cleanup();
	
	bool decodeNextFrame();
	bool findNextHeader();
	bool checkFrameHeader(mad_header* header) const;
	
	void createInputBuffer();
	void clearInputBuffer();
	
	mad_stream*   madStream;
	mad_frame*    madFrame;
	mad_synth*    madSynth;
	mad_timer_t*  madTimer;
	
private:
	QFile m_inputFile;
	bool m_madStructuresInitialized;
	unsigned char* m_inputBuffer;
	bool m_bInputError;
	
	int m_channels;
	int m_sampleRate;
};


K3bMad::K3bMad()
  : m_madStructuresInitialized(false),
    m_bInputError(false)
{
	madStream = new mad_stream;
	madFrame  = new mad_frame;
	madSynth  = new mad_synth;
	madTimer  = new mad_timer_t;
	
	m_inputBuffer = 0;
}


K3bMad::~K3bMad()
{
	cleanup();
	
	delete madStream;
	delete madFrame;
	delete madSynth;
	delete madTimer;
	
	clearInputBuffer();
}


void K3bMad::createInputBuffer()
{
	if (!m_inputBuffer) {
		//
		// we allocate additional MAD_BUFFER_GUARD bytes to always be able to append the
		// zero bytes needed for decoding the last frame.
		//
		m_inputBuffer = new unsigned char[INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD];
		memset(m_inputBuffer, 0, INPUT_BUFFER_SIZE+MAD_BUFFER_GUARD);
	}
}


void K3bMad::clearInputBuffer()
{
	if (m_inputBuffer) {
		delete [] m_inputBuffer;
		m_inputBuffer = 0;
	}
}


bool K3bMad::open(const QString& filename)
{
	cleanup();
	
	m_bInputError = false;
	m_channels = m_sampleRate = 0;
	
	m_inputFile.setFileName(filename);
	
	if (!m_inputFile.open(QIODevice::ReadOnly)) {
		PERROR("could not open file %s", m_inputFile.fileName().toUtf8().data());
		return false;
	}
	
	initMad();
	
	return true;
}


bool K3bMad::inputError() const
{
	return m_bInputError;
}


bool K3bMad::fillStreamBuffer()
{
	/* The input bucket must be filled if it becomes empty or if
	* it's the first execution of the loop.
	*/
	if (madStream->buffer == 0 || madStream->error == MAD_ERROR_BUFLEN) {
		if (eof()) {
			return false;
		}
		
		if (!m_inputBuffer) {
			createInputBuffer();
		}
		
		long readSize, remaining;
		unsigned char* readStart;
		
		if (madStream->next_frame != 0) {
			remaining = madStream->bufend - madStream->next_frame;
			memmove(m_inputBuffer, madStream->next_frame, remaining);
			readStart = m_inputBuffer + remaining;
			readSize = INPUT_BUFFER_SIZE - remaining;
		}
		else {
			readSize  = INPUT_BUFFER_SIZE;
			readStart = m_inputBuffer;
			remaining = 0;
		}
		
		// Fill-in the buffer. 
		long result = m_inputFile.read((char*)readStart, readSize);
		if (result < 0) {
			//kdDebug() << "(K3bMad) read error on bitstream)" << endl;
			m_bInputError = true;
			return false;
		}
		else if (result == 0) {
			//kdDebug() << "(K3bMad) end of input stream" << endl;
			return false;
		}
		else {
			readStart += result;
			
			if (eof()) {
				//kdDebug() << "(K3bMad::fillStreamBuffer) MAD_BUFFER_GUARD" << endl;
				memset(readStart, 0, MAD_BUFFER_GUARD);
				result += MAD_BUFFER_GUARD;
			}
			
			// Pipe the new buffer content to libmad's stream decoder facility.
			mad_stream_buffer(madStream, m_inputBuffer, result + remaining);
			madStream->error = MAD_ERROR_NONE;
		}
	}
	
	return true;
}


bool K3bMad::skipTag()
{
	// skip the tag at the beginning of the file
	m_inputFile.seek(0);
	
	//
	// now check if the file starts with an id3 tag and skip it if so
	//
	char buf[4096];
	int bufLen = 4096;
	if (m_inputFile.read(buf, bufLen) < bufLen) {
		//kdDebug() << "(K3bMad) unable to read " << bufLen << " bytes from " 
		//	      << m_inputFile.name() << endl;
		return false;
	}
	
	if ((buf[0] == 'I' && buf[1] == 'D' && buf[2] == '3') &&
	   ((unsigned short)buf[3] < 0xff && (unsigned short)buf[4] < 0xff)) {
		// do we have a footer?
		bool footer = (buf[5] & 0x10);
		
		// the size is saved as a synched int meaning bit 7 is always cleared to 0
		unsigned int size =
			( (buf[6] & 0x7f) << 21 ) |
			( (buf[7] & 0x7f) << 14 ) |
			( (buf[8] & 0x7f) << 7) |
			(buf[9] & 0x7f);
		unsigned int offset = size + 10;
		
		if (footer) {
			offset += 10;
		}
		
		// skip the id3 tag
		if (!m_inputFile.seek(offset)) {
			PERROR("Couldn't seek to %u in %s", offset, m_inputFile.fileName().toUtf8().data());
			return false;
		}
	}
	else {
		// reset file
		return m_inputFile.seek(0);
	}
	
	return true;
}


bool K3bMad::seekFirstHeader()
{
	//
	// A lot of mp3 files start with a lot of junk which confuses mad.
	// We "allow" an mp3 file to start with at most 1 KB of junk. This is just
	// some random value since we do not want to search the hole file. That would
	// take way to long for non-mp3 files.
	//
	bool headerFound = findNextHeader();
	qint64 inputPos = streamPos();
	while (!headerFound &&
	   !m_inputFile.atEnd() && 
	   streamPos() <= inputPos+1024) {
		headerFound = findNextHeader();
	}
	
	// seek back to the begin of the frame
	if (headerFound) {
		int streamSize = madStream->bufend - madStream->buffer;
		int bytesToFrame = madStream->this_frame - madStream->buffer;
		m_inputFile.seek(m_inputFile.pos() - streamSize + bytesToFrame);
		
		//kdDebug() << "(K3bMad) found first header at " << m_inputFile.pos() << endl;
	}
	
	// reset the stream to make sure mad really starts decoding at out seek position
	mad_stream_finish(madStream);
	mad_stream_init(madStream);
	
	return headerFound;
}


bool K3bMad::eof() const
{ 
	return m_inputFile.atEnd();
}


qint64 K3bMad::inputPos() const
{
	return m_inputFile.pos();
}


qint64 K3bMad::streamPos() const
{
	return inputPos() - (madStream->bufend - madStream->this_frame + 1);
}


bool K3bMad::inputSeek(qint64 pos)
{
	return m_inputFile.seek(pos);
}


void K3bMad::initMad()
{
	if (!m_madStructuresInitialized) {
		mad_stream_init(madStream);
		mad_timer_set(madTimer, 0, 0, 0);
		mad_frame_init(madFrame);
		mad_synth_init(madSynth);
		
		m_madStructuresInitialized = true;
	}
}


void K3bMad::cleanup()
{
	if (m_inputFile.isOpen()) {
		//kdDebug() << "(K3bMad) cleanup at offset: " 
		//	      << "Input file at: " << m_inputFile.pos() << " "
		//	      << "Input file size: " << m_inputFile.size() << " "
		//	      << "stream pos: " 
		//	      << ( m_inputFile.pos() - (madStream->bufend - madStream->this_frame + 1) )
		//	      << endl;
		m_inputFile.close();
	}
	
	if (m_madStructuresInitialized) {
		mad_frame_finish(madFrame);
		mad_synth_finish(madSynth);
		mad_stream_finish(madStream);
	}
	
	m_madStructuresInitialized = false;
}


//
// LOSTSYNC could happen when mad encounters the id3 tag...
//
bool K3bMad::findNextHeader()
{
	if (!fillStreamBuffer()) {
		return false;
	}
	
	//
	// MAD_RECOVERABLE == true:  frame was read, decoding failed (about to skip frame)
	// MAD_RECOVERABLE == false: frame was not read, need data
	//
	
	if (mad_header_decode( &madFrame->header, madStream ) < 0) {
		if (MAD_RECOVERABLE(madStream->error) ||
		   madStream->error == MAD_ERROR_BUFLEN) {
			return findNextHeader();
		}
		else
		//      kdDebug() << "(K3bMad::findNextHeader) error: " << mad_stream_errorstr( madStream ) << endl;
		
		// FIXME probably we should not do this here since we don't do it
		// in the frame decoding
		//     if(!checkFrameHeader(&madFrame->header))
		//       return findNextHeader();
		
		return false;
	}
	
	if (!m_channels) {
		m_channels = MAD_NCHANNELS(&madFrame->header);
		m_sampleRate = madFrame->header.samplerate;
	}
	
	mad_timer_add(madTimer, madFrame->header.duration);
	
	return true;
}


bool K3bMad::decodeNextFrame()
{
	if (!fillStreamBuffer()) {
		return false;
	}
	
	if (mad_frame_decode(madFrame, madStream) < 0) {
		if (MAD_RECOVERABLE(madStream->error) ||
		    madStream->error == MAD_ERROR_BUFLEN) {
			return decodeNextFrame();
		}
	
		return false;
	}
	
	if (!m_channels) {
		m_channels = MAD_NCHANNELS(&madFrame->header);
		m_sampleRate = madFrame->header.samplerate;
	}
	
	mad_timer_add(madTimer, madFrame->header.duration);
	
	return true;
}


//
// This is from the arts mad decoder
//
bool K3bMad::checkFrameHeader(mad_header* header) const
{
	int frameSize = MAD_NSBSAMPLES(header) * 32;
	
	if (frameSize <= 0) {
		return false;
	}
	
	if (m_channels && m_channels != MAD_NCHANNELS(header)) {
		return false;
	}
	
	return true;
}



class MadAudioReader::MadDecoderPrivate
{
public:
	MadDecoderPrivate()
	{
		outputBuffers = 0;
		outputPos = 0;
		outputSize = 0;
		overflowSize = 0;
		overflowStart = 0;
		
		mad_header_init( &firstHeader );
	}
	
	K3bMad* handle;
	
	QVector<unsigned long long> seekPositions;
	
	bool bOutputFinished;
	
	audio_sample_t** outputBuffers;
	nframes_t	outputPos;
	nframes_t	outputSize;
	
	audio_sample_t** overflowBuffers;
	nframes_t	overflowSize;
	nframes_t	overflowStart;
	
	// the first frame header for technical info
	mad_header firstHeader;
	bool vbr;
};


MadAudioReader::MadAudioReader(QString filename)
 : AbstractAudioReader(filename)
{
	d = new MadDecoderPrivate();
	d->handle = new K3bMad();
	
	initDecoderInternal();
	
	m_nframes = countFrames();
	
	switch( d->firstHeader.mode ) {
		case MAD_MODE_SINGLE_CHANNEL:
			m_channels = 1;
			break;
		case MAD_MODE_DUAL_CHANNEL:
		case MAD_MODE_JOINT_STEREO:
		case MAD_MODE_STEREO:
			m_channels = 2;
	}
	
	if (m_nframes <= 0) {
		d->handle->cleanup();
		delete d->handle;
		delete d;
		d = 0;
		return;
	}
	
	m_rate = d->firstHeader.samplerate;
	m_length = TimeRef(m_nframes, m_rate);
	
	d->overflowBuffers = 0;
	
	seek_private(0);
	clear_buffers();
}


MadAudioReader::~MadAudioReader()
{
	if (d) {
		d->handle->cleanup();
		delete d->handle;
		clear_buffers();
		delete d;
	}
}


void MadAudioReader::create_buffers()
{
	if (!d->overflowBuffers) {
		d->overflowBuffers = new audio_sample_t*[m_channels];
		for (int chan = 0; chan < m_channels; chan++) {
			d->overflowBuffers[chan] = new audio_sample_t[1152];
		}
	}
}
	

void MadAudioReader::clear_buffers()
{
	if (d->overflowBuffers) {
		for (int chan = 0; chan < m_channels; chan++) {
			delete [] d->overflowBuffers[chan];
		}
		delete [] d->overflowBuffers;
		d->overflowBuffers = 0;
		d->overflowStart = 0;
		d->overflowSize = 0;
	}
	
	if (d && d->handle) {
		d->handle->clearInputBuffer();
	}
}


bool MadAudioReader::can_decode(QString filename)
{
	if (!libmad_is_present) {
		return false;
	}
	
	//
	// HACK:
	//
	// I am simply no good at this and this detection code is no good as well
	// It always takes waves for mp3 files so we introduce this hack to
	// filter out wave files. :(
	//
	QFile f(filename);
	if (!f.open( QIODevice::ReadOnly)) {
		return false;
	}
	
	char buffer[12];
	if (f.read(buffer, 12) != 12) {
		return false;
	}
	if (!qstrncmp(buffer, "RIFF", 4) && !qstrncmp(buffer + 8, "WAVE", 4)) {
		return false;
	}
	f.close();
	
	
	K3bMad handle;
	if (!handle.open(filename)) {
		return false;
	}
	handle.skipTag();
	if (!handle.seekFirstHeader()) {
		return false;
	}
	if (handle.findNextHeader()) {
		int c = MAD_NCHANNELS(&handle.madFrame->header);
		int layer = handle.madFrame->header.layer;
		unsigned int s = handle.madFrame->header.samplerate;
		
		//
		// find 4 more mp3 headers (random value since 2 was not enough)
		// This way we get most of the mp3 files while sorting out
		// for example wave files.
		//
		int cnt = 1;
		while (handle.findNextHeader()) {
			// compare the found headers
			if (MAD_NCHANNELS(&handle.madFrame->header) == c &&
			    handle.madFrame->header.layer == layer &&
			    handle.madFrame->header.samplerate == s) {
				// only support layer III for now since otherwise some wave files
				// are taken for layer I
				if (++cnt >= 5) {
					//stdout << "(MadDecoder) valid mpeg 1 layer " << layer 
					//<< " file with " << c << " channels and a samplerate of "
					//<< s << endl;
					return (layer == MAD_LAYER_III);
				}
			}
			else {
				break;
			}
		}
	}
	
	//PERROR("unsupported format: %s",QS_C(filename));
	
	return false;
}


bool MadAudioReader::seek_private(nframes_t start)
{
	Q_ASSERT(d);
	
	if (start >= m_nframes) {
		return false;
	}
	
	//
	// we need to reset the complete mad stuff 
	//
	if (!initDecoderInternal()) {
		return false;
	}
	
	//
	// search a position
	// This is all hacking, I don't really know what I am doing here... ;)
	//
	double mp3FrameSecs = static_cast<double>(d->firstHeader.duration.seconds) + static_cast<double>(d->firstHeader.duration.fraction) / static_cast<double>(MAD_TIMER_RESOLUTION);
	
	double posSecs = static_cast<double>(start) / m_rate;
	
	// seekPosition to seek after frame i
	unsigned int frame = static_cast<unsigned int>(posSecs / mp3FrameSecs);
	nframes_t frameOffset = (nframes_t)(start - (frame * mp3FrameSecs * m_rate + 0.5));
	
	// K3b source: Rob said: 29 frames is the theoretically max frame reservoir limit
	// (whatever that means...) it seems that mad needs at most 29 frames to get ready
	//
	// Ben says: It looks like Rob (the author of MAD) implies here:
	//    http://www.mars.org/mailman/public/mad-dev/2001-August/000321.html
	// that 3 frames (1 + 2 extra) is enough... much faster, and seems to work fine...
	unsigned int frameReservoirProtect = (frame > 3 ? 3 : frame);
	
	frame -= frameReservoirProtect;
	
	// seek in the input file behind the already decoded data
	d->handle->inputSeek( d->seekPositions[frame] );
	
	// decode some frames ignoring MAD_ERROR_BADDATAPTR errors
	unsigned int i = 1;
	while (i <= frameReservoirProtect) {
		d->handle->fillStreamBuffer();
		if (mad_frame_decode( d->handle->madFrame, d->handle->madStream)) {
			if (MAD_RECOVERABLE( d->handle->madStream->error)) {
				if (d->handle->madStream->error == MAD_ERROR_BUFLEN) {
					continue;
				}
				else if (d->handle->madStream->error != MAD_ERROR_BADDATAPTR) {
					//kdDebug() << "(K3bMadDecoder) Seeking: recoverable mad error ("
					//<< mad_stream_errorstr(d->handle->madStream) << ")" << endl;
					continue;
				}
				else {
					//kdDebug() << "(K3bMadDecoder) Seeking: ignoring (" 
					//<< mad_stream_errorstr(d->handle->madStream) << ")" << endl;
				}
			}
			else {
				return false;
			}
		}
		
		if (i == frameReservoirProtect) {  // synth only the last frame (Rob said so ;)
			mad_synth_frame( d->handle->madSynth, d->handle->madFrame );
		}
		
		++i;
	}
	
	d->overflowStart = 0;
	d->overflowSize = 0;
	
	// Seek to exact traverso frame, within this mp3 frame
	if (frameOffset > 0) {
		//printf("seekOffset: %lu (start: %lu)\n", frameOffset, start);
		d->outputBuffers = 0; // Zeros so that we write to overflow
		d->outputSize = 0;
		d->outputPos = 0;
		createPcmSamples(d->handle->madSynth);
		d->overflowStart = frameOffset;
		d->overflowSize -= frameOffset;
	}
	
	return true;
}


bool MadAudioReader::initDecoderInternal()
{
	d->handle->cleanup();
	
	d->bOutputFinished = false;
	
	if (!d->handle->open(m_fileName)) {
		return false;
	}
	
	if (!d->handle->skipTag()) {
		return false;
	}
	
	if (!d->handle->seekFirstHeader()) {
		return false;
	}
	
	return true;
}


unsigned long MadAudioReader::countFrames()
{
	unsigned long frames = 0;
	bool error = false;
	d->vbr = false;
	bool bFirstHeaderSaved = false;
	
	d->seekPositions.clear();
	
	while (!error && d->handle->findNextHeader()) {
		if (!bFirstHeaderSaved) {
			bFirstHeaderSaved = true;
			d->firstHeader = d->handle->madFrame->header;
		}
		else if (d->handle->madFrame->header.bitrate != d->firstHeader.bitrate) {
			d->vbr = true;
		}
		//
		// position in stream: position in file minus the not yet used buffer
		//
		unsigned long long seekPos = d->handle->inputPos() - 
		(d->handle->madStream->bufend - d->handle->madStream->this_frame + 1);
		
		// save the number of bytes to be read to decode i-1 frames at position i
		// in other words: when seeking to seekPos the next decoded frame will be i
		d->seekPositions.append(seekPos);
	}
	
	if (!d->handle->inputError() && !error) {
		frames =  d->firstHeader.samplerate * (d->handle->madTimer->seconds + (unsigned long)(
			(float)d->handle->madTimer->fraction/(float)MAD_TIMER_RESOLUTION));
	}

	d->handle->cleanup();
	
	return frames;
}


nframes_t MadAudioReader::read_private(DecodeBuffer* buffer, nframes_t frameCount)
{
	d->outputBuffers = buffer->destination;
	d->outputSize = frameCount;
	d->outputPos = 0;
	
	bool bOutputBufferFull = false;
	
	// Deal with existing overflow
	if (d->overflowSize > 0) {
		if (d->overflowSize < frameCount) {
			//printf("output all %d overflow samples\n", d->overflowSize);
			for (int chan = 0; chan < m_channels; chan++) {
				memcpy(d->outputBuffers[chan], d->overflowBuffers[chan] + d->overflowStart, d->overflowSize * sizeof(audio_sample_t));
			}
			d->outputPos += d->overflowSize;
			d->overflowSize = 0;
			d->overflowStart = 0;
		}
		else {
			//printf("output %d overflow frames, returned from overflow\n", frameCount);
			for (int chan = 0; chan < m_channels; chan++) {
				memcpy(d->outputBuffers[chan], d->overflowBuffers[chan] + d->overflowStart, frameCount * sizeof(audio_sample_t));
			}
			d->overflowSize -= frameCount;
			d->overflowStart += frameCount;
			return frameCount;
		}
	}
	
	while (!bOutputBufferFull && d->handle->fillStreamBuffer()) {
		// a mad_synth contains of the data of one mad_frame
		// one mad_frame represents a mp3-frame which is always 1152 samples
		// for us that means we need 1152 samples per channel of output buffer
		// for every frame
		if (d->outputPos >= d->outputSize) {
			bOutputBufferFull = true;
		}
		else if (d->handle->decodeNextFrame()) {
			// 
			// Once decoded the frame is synthesized to PCM samples. No errors
			// are reported by mad_synth_frame();
			//
			mad_synth_frame( d->handle->madSynth, d->handle->madFrame );
			
			// this fills the output buffer
			if (!createPcmSamples(d->handle->madSynth)) {
				PERROR("createPcmSamples");
				return 0;
			}
		}
		else if (d->handle->inputError()) {
			PERROR("inputError");
			return 0;
		}
	}
	
	nframes_t framesWritten = d->outputPos;
	
	return framesWritten;
}


bool MadAudioReader::createPcmSamples(mad_synth* synth)
{
	audio_sample_t	**writeBuffers = d->outputBuffers;
	int		offset = d->outputPos;
	nframes_t	nframes = synth->pcm.length;
	bool		overflow = false;
	nframes_t	i;
	
	if (!d->overflowBuffers) {
		create_buffers();
	}
	
	if (writeBuffers && (m_readPos + d->outputPos + nframes) > m_nframes) {
		nframes = m_nframes - (m_readPos + offset);
	}
	
	// now create the output
	for (i = 0; i < nframes; i++) {
		if (overflow == false && d->outputPos + i >= d->outputSize) {
			writeBuffers = d->overflowBuffers;
			offset = 0 - i;
			overflow = true;
		}
		
		/* Left channel */
		writeBuffers[0][offset + i] = mad_f_todouble(synth->pcm.samples[0][i]);
		
		/* Right channel. If the decoded stream is monophonic then no right channel
		*/
		if (synth->pcm.channels == 2) {
			writeBuffers[1][offset + i] = mad_f_todouble(synth->pcm.samples[1][i]);
		}
	} // pcm conversion
	
	if (overflow) {
		d->overflowSize = i + offset;
		d->overflowStart = 0;
		d->outputPos -= offset; // i was stored here when we switched to writing to overflow
		//printf("written: %d (overflow: %u)\n",  nframes - d->overflowSize, d->overflowSize);
	}
	else {
		d->outputPos += i;
		//printf("written: %d (os=%lu)\n",  i, d->overflowSize);
	}
	
	return true;
}


