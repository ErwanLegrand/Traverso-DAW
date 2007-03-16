<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS><TS version="1.1">
<context>
    <name>AlsaDevicesPage</name>
    <message>
        <location filename="src/traverso/ui/AlsaDevicesPage.ui" line="13"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AlsaDevicesPage.ui" line="25"/>
        <source>ALSA Device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AlsaDevicesPage.ui" line="45"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Device:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The real or virtual ALSA device to be used.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A real device is the audiocard installed in your system.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;A virtual device is one created in the .asoundrc file, often located in your home folder.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;If unsure, use either the default device, this will use the audiodevice configured by your distribution, or the device that names your audio card.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;In the latter case, please make sure no application uses the audiocard, else the driver won&apos;t be able to initialize!&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;For more info see chapter 3.1: &quot;The Driver Backend&quot; of the User Manual&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AlsaDevicesPage.ui" line="61"/>
        <source>Device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AlsaDevicesPage.ui" line="81"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Number of Periods:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Audio is managed in small chunks called periods. This value determines how many of these chuncks are to be used by the driver of the audiocard.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The default should work just fine, and gives optimal latency behavior.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;However, some (buggy) alsa drivers don&apos;t work correctly with the default of 2, if you experience very choppy audio, please try to use 3 periods.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AlsaDevicesPage.ui" line="91"/>
        <source>Nr. of periods</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AlsaDevicesPage.ui" line="99"/>
        <source>2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AlsaDevicesPage.ui" line="104"/>
        <source>3</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AlsaDriver</name>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="172"/>
        <source>ALSA Driver: The playback device %1 is already in use. Please stop the application using it and run Traverso again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="177"/>
        <source>ALSA Driver: You do not have permission to open the audio device %1 for playback</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="181"/>
        <source>snd_pcm_open(playback_handle, ..) failed with unknown error type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="197"/>
        <source>ALSA Driver: The capture device %1 is already in use. Please stop the application using it and run Traverso again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="202"/>
        <source>ALSA Driver: You do not have permission to open the audio device %1 for capture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="206"/>
        <source>ALSA Driver: snd_pcm_open(capture_handle, ...) failed with unknown error type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="222"/>
        <source>ALSA Driver: Cannot open PCM device %1 for playback. Falling back to capture-only mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="237"/>
        <source>ALSA: Cannot open PCM device %1 for capture. Falling back to playback-only mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AlsaDriver.cpp" line="555"/>
        <source>ALSA Driver: Unable to configure hardware, is it in use by another application?</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioClip</name>
    <message>
        <location filename="src/core/AudioClip.cpp" line="480"/>
        <source>Unable to Record to Track</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.cpp" line="482"/>
        <source>AudioDevice doesn&apos;t have this Capture Bus: %1 (Track %2)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.cpp" line="527"/>
        <source>Toggle Mute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.cpp" line="570"/>
        <source>Copy of - </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.cpp" line="828"/>
        <source>Normalization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.cpp" line="829"/>
        <source>Set Normalization level:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.h" line="47"/>
        <source>Mute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.h" line="48"/>
        <source>Fade In: Reset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.h" line="49"/>
        <source>Fade Out: Reset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.h" line="50"/>
        <source>Fade: Reset both</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.h" line="51"/>
        <source>Fade In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.h" line="52"/>
        <source>Fade Out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.h" line="53"/>
        <source>Normalize</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClip.h" line="54"/>
        <source>Normalize: reset</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioClipExternalProcessing</name>
    <message>
        <location filename="src/commands/AudioClipExternalProcessing.cpp" line="59"/>
        <source>Clip Processing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/AudioClipExternalProcessing.cpp" line="60"/>
        <source>Enter sox command</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioClipManager</name>
    <message>
        <location filename="src/core/AudioClipManager.cpp" line="185"/>
        <source>Selection: Add Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClipManager.cpp" line="192"/>
        <source>Selection: Remove Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClipManager.h" line="39"/>
        <source>Selection: Invert</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClipManager.cpp" line="205"/>
        <source>Remove Clip(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClipManager.h" line="37"/>
        <source>Selection: Select all</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClipManager.h" line="38"/>
        <source>Selection: Deselect all</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/AudioClipManager.h" line="40"/>
        <source>Selection: Delete selected</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioClipView</name>
    <message>
        <location filename="src/traverso/songcanvas/AudioClipView.h" line="42"/>
        <source>Move Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/AudioClipView.h" line="43"/>
        <source>Move Edge</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/AudioClipView.h" line="44"/>
        <source>Split</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/AudioClipView.h" line="45"/>
        <source>Fade In/Out</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioDevice</name>
    <message>
        <location filename="src/engine/AudioDevice.cpp" line="143"/>
        <source>No Driver Loaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AudioDevice.cpp" line="556"/>
        <source>No Device Configured</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/AudioDevice.cpp" line="677"/>
        <source>The Jack server has been shutdown!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioDriverPage</name>
    <message>
        <location filename="src/traverso/dialogs/settings/Pages.cpp" line="50"/>
        <source>Driver Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/Pages.cpp" line="53"/>
        <source>Driver:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/Pages.cpp" line="69"/>
        <source>Restart Driver</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioSourceManager</name>
    <message>
        <location filename="src/core/AudioSourceManager.cpp" line="180"/>
        <source>Failed to initialize ReadSource, removing from database: %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioSourcesManagerWidget</name>
    <message>
        <location filename="src/traverso/ui/AudioSourcesManagerWidget.ui" line="16"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AudioSourcesManagerWidget.ui" line="75"/>
        <source>Remove sources</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AudioSourcesManagerWidget.ui" line="87"/>
        <source>Remove source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AudioSourcesManagerWidget.ui" line="94"/>
        <source>Remove all sources</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AudioSourcesManagerWidget.ui" line="101"/>
        <source>Remove unused sources</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/AudioSourcesManagerWidget.ui" line="111"/>
        <source>AudioSources</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>BehaviorConfigPage</name>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="13"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="25"/>
        <source>Project Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="37"/>
        <source>Load last used project at startup</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="52"/>
        <source>Default project directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="89"/>
        <source>On project close</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="104"/>
        <source>Save Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="114"/>
        <source>Ask</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="121"/>
        <source>Don&apos;t save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="148"/>
        <source>New Song options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/BehaviorConfigPage.ui" line="168"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Bitstream Vera Sans; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Number of tracks&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/Pages.cpp" line="380"/>
        <source>Select default project dir</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ClipSelection</name>
    <message>
        <location filename="src/commands/ClipSelection.cpp" line="38"/>
        <source>Selection: Remove Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/ClipSelection.cpp" line="40"/>
        <source>Selection: Add Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/ClipSelection.cpp" line="42"/>
        <source>Select Clip</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Config</name>
    <message>
        <location filename="src/core/Config.cpp" line="97"/>
        <source>Choose an existing or create a new Project Directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Config.cpp" line="113"/>
        <source>Traverso - Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Config.cpp" line="102"/>
        <source>Using existing Project directory: %1
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Config.cpp" line="107"/>
        <source>Traverso - Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Config.cpp" line="109"/>
        <source>Unable to create Project directory! 
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Config.cpp" line="109"/>
        <source>Please check permission for this directory: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Config.cpp" line="114"/>
        <source>Created new Project directory for you here: %1
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CopyClip</name>
    <message>
        <location filename="src/commands/CopyClip.cpp" line="40"/>
        <source>Copy clip %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CorrelationMeterView</name>
    <message>
        <location filename="src/traverso/CorrelationMeterWidget.h" line="65"/>
        <source>Toggle display range</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CurveView</name>
    <message>
        <location filename="src/traverso/songcanvas/CurveView.cpp" line="450"/>
        <source>Drag Node</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/CurveView.h" line="38"/>
        <source>New node</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/CurveView.h" line="39"/>
        <source>Remove node</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/CurveView.h" line="40"/>
        <source>Move node</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Dialog</name>
    <message>
        <location filename="src/traverso/ui/PluginSelector.ui" line="16"/>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/PluginSelector.ui" line="60"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/PluginSelector.ui" line="67"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>DigitalClock</name>
    <message>
        <location filename="src/traverso/Interface.cpp" line="613"/>
        <source>Digital Clock</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>DriverConfigPage</name>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="13"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="25"/>
        <source>Configure driver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="45"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Duplex mode:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Defines if both the Playback and Capture buses of your soundcard are to be used, or only the Playback or Capture bus.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="52"/>
        <source>Duplex mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="60"/>
        <source>Full</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="65"/>
        <source>Playback</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="70"/>
        <source>Capture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="88"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Sample rate:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The sample rate used by the audio card.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;New projects will use this samplerate as the project&apos;s sample rate on creation.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="97"/>
        <source>Sample rate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="105"/>
        <source>22050</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="110"/>
        <source>32000</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="115"/>
        <source>44100</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="120"/>
        <source>48000</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="125"/>
        <source>88200</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="130"/>
        <source>96000</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="148"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Buffer latency:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The latency introduced by the size of the audio buffers.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Some people need low latencies, if you don&apos;t need it, or don&apos;t know what it means, please leave the default setting!&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/DriverConfigPage.ui" line="157"/>
        <source>Buffer latency (ms)</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>DriverInfo</name>
    <message>
        <location filename="src/traverso/widgets/InfoWidgets.cpp" line="159"/>
        <source>Click to configure audiodevice</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ExportWidget</name>
    <message>
        <location filename="src/traverso/ExportWidget.cpp" line="226"/>
        <source>No project loaded, to export a project, load it first!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ExportWidget.cpp" line="125"/>
        <source>Unable to create export directory! Please check permissions for this directory: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ExportWidget.cpp" line="230"/>
        <source>Choose/create an export directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="38"/>
        <source>Project export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="58"/>
        <source>General options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="73"/>
        <source>Export directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="106"/>
        <source>...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="113"/>
        <source>Song(s) to render</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="123"/>
        <source>Selection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="130"/>
        <source>Current</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="140"/>
        <source>All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="158"/>
        <source>Export specifications</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="178"/>
        <source>Audio type</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="197"/>
        <source>Channels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="212"/>
        <source>Bitdepth</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="228"/>
        <source>Sample rate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="275"/>
        <source>Start</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="282"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="299"/>
        <source>Export status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="324"/>
        <source>Progress of Song:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="331"/>
        <source>-</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="338"/>
        <source>Stop</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ExportWidget.ui" line="345"/>
        <source>Overall Progress</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FadeContextDialog</name>
    <message>
        <location filename="src/traverso/FadeContextDialog.cpp" line="39"/>
        <source>Fade Editor</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FadeCurve</name>
    <message>
        <location filename="src/core/FadeCurve.h" line="40"/>
        <source>Toggle bypass</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/FadeCurve.h" line="41"/>
        <source>Set Mode</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/FadeCurve.h" line="42"/>
        <source>Reset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/FadeCurve.h" line="43"/>
        <source>Toggle raster</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FadeView</name>
    <message>
        <location filename="src/traverso/songcanvas/FadeView.h" line="37"/>
        <source>Bend</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/FadeView.h" line="38"/>
        <source>Strength</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>InfoToolBar</name>
    <message>
        <location filename="src/traverso/widgets/InfoWidgets.cpp" line="543"/>
        <source>Main Toolbar</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>InputEngine</name>
    <message>
        <location filename="src/core/InputEngine.cpp" line="302"/>
        <source>Command Plugin %1 not found!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/InputEngine.cpp" line="305"/>
        <source>Plugin %1 doesn&apos;t implement Command %2</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Interface</name>
    <message>
        <location filename="src/traverso/Interface.cpp" line="79"/>
        <source>History</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="88"/>
        <source>AudioSources</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="100"/>
        <source>Correlation Meter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="107"/>
        <source>FFT Spectrum</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="234"/>
        <source>Traverso %1, making use of Qt %2

Traverso, a Multitrack audio recording and editing program.

Traverso uses a very powerfull interface concept, which makes recording
and editing audio much quicker and a pleasure to do!
See for more info the Help file

Traverso is brought to you by the author, R. Sijrier, and all the people from Free Software world
who made important technologies on which Traverso is based (Gcc, Qt, Xorg, Linux, and so on)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="235"/>
        <source>About Traverso</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="274"/>
        <source>&amp;File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="276"/>
        <source>Open / Create</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="283"/>
        <source>&amp;Quit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="288"/>
        <source>&amp;Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="290"/>
        <source>&amp;Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="296"/>
        <source>Manage Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="305"/>
        <source>Export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="311"/>
        <source>&amp;Views</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="325"/>
        <source>Main Toolbar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="327"/>
        <source>System Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="330"/>
        <source>&amp;Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="335"/>
        <source>&amp;HandBook</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="339"/>
        <source>&amp;About Traverso</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="343"/>
        <source>OpenGL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="349"/>
        <source>Undo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/Interface.cpp" line="354"/>
        <source>Redo</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>JackDriver</name>
    <message>
        <location filename="src/engine/JackDriver.cpp" line="118"/>
        <source>Jack Driver: Couldn&apos;t connect to the jack server, is jack running?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/JackDriver.cpp" line="214"/>
        <source>Jack Driver: Connected successfully to the jack server!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>KeyboardConfigPage</name>
    <message>
        <location filename="src/traverso/ui/KeyboardConfigPage.ui" line="13"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/KeyboardConfigPage.ui" line="41"/>
        <source>Configure Keyboard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/KeyboardConfigPage.ui" line="61"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Double fact timeout:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The maximum time in miliseconds between 2 key presses to determine if the 2 key presses are a double fact ( &amp;lt;&amp;lt; K &amp;gt;&amp;gt; or &amp;lt;&amp;lt; KK &amp;gt;&amp;gt;) or 2 individual key presses ( a &amp;lt; K &amp;gt; and &amp;lt; K &amp;gt; action, or &amp;lt; KK &amp;gt; and &amp;lt; KK &amp;gt; action for example).&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Experienced users can set this value as low as 150 ms, if you don&apos;t have much experience yet, please leave the default of 200 ms.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;For more information, see chapter 7: Key Actions. of the User Manual&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/KeyboardConfigPage.ui" line="72"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Bitstream Vera Sans; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Double fact timeout (ms)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/KeyboardConfigPage.ui" line="108"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Hold timeout:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The maximum time to consider a pressed key a hold key fact, like [ K ] or [ KK ].&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The shorter this time, the sooner a pressed key will be detected as a hold action. &lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Experienced users can set this value as low as 150 ms, if you don&apos;t have much experience yet, please leave the default of 200 ms.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;For more information, see chapter 7: &quot;Key Actions&quot; of the User Manual.&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/KeyboardConfigPage.ui" line="121"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Bitstream Vera Sans; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Hold timeout (ms)&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>MemoryConfigPage</name>
    <message>
        <location filename="src/traverso/ui/MemoryConfigPage.ui" line="21"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/MemoryConfigPage.ui" line="33"/>
        <source>Audio file buffering</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/MemoryConfigPage.ui" line="53"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Sans Serif&apos;; font-size:9pt; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-weight:600;&quot;&gt;Read / Write buffer size:&lt;/span&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The amount of audio data that can be stored in the read / write buffers in seconds.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;The default value of 1 second should do just fine.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;However, if you&apos;re tight on memory, you can make this value lower.&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;If you experience buffer underruns when the hard disk bandwidth is (almost) saturated, or when buffer underruns happen regularly due the hard disk can&apos;t keep up for some reason, you can try a larger value, like 1.5 or 2.0 seconds.&lt;/p&gt;
&lt;p style=&quot;-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;/p&gt;
&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Keep in mind that when using a larger buffer, it will take considerably more time to move (i.e. seeking) the playhead to another positions, since all the buffers (one for each audioclip * channel count) need to be refilled!&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/MemoryConfigPage.ui" line="66"/>
        <source>Read / Write buffer size (seconds)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/MemoryConfigPage.ui" line="116"/>
        <source>info icon</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/MemoryConfigPage.ui" line="131"/>
        <source>Changing the buffer size only will take 
into effect after (re)loading a project.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>PlayHeadInfo</name>
    <message>
        <location filename="src/traverso/widgets/InfoWidgets.cpp" line="456"/>
        <source>Playhead position&lt;br /&gt;&lt;br /&gt;Click to change Playhead behavior</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>PluginSelectorDialog</name>
    <message>
        <location filename="src/traverso/PluginSelectorDialog.cpp" line="101"/>
        <source>Plugin initialization failed!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/PluginSelectorDialog.ui" line="16"/>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/PluginSelectorDialog.ui" line="60"/>
        <source>OK</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/PluginSelectorDialog.ui" line="67"/>
        <source>Cancel</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>PluginView</name>
    <message>
        <location filename="src/traverso/songcanvas/PluginView.h" line="41"/>
        <source>Settings...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/PluginView.h" line="42"/>
        <source>Remove</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Project</name>
    <message>
        <location filename="src/core/Project.cpp" line="183"/>
        <source>Project &apos;%1&apos; loaded</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Project.cpp" line="227"/>
        <source>Project &apos;%1&apos; saved </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Project.cpp" line="230"/>
        <source>Could not open project properties file for writing! (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Project.cpp" line="271"/>
        <source>Song %1 added</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Project.cpp" line="293"/>
        <source>Song &apos;%1&apos; doesn&apos;t exist!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Project.cpp" line="340"/>
        <source>Song %1 removed</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectInfoWidget</name>
    <message>
        <location filename="src/traverso/ui/ProjectInfoWidget.ui" line="16"/>
        <source>Project Information Widget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectInfoWidget.ui" line="60"/>
        <source>Bitdepth</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectInfoWidget.ui" line="183"/>
        <source>-</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectInfoWidget.ui" line="135"/>
        <source>Rate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectInfoWidget.ui" line="201"/>
        <source>Songs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectInfoWidget.ui" line="231"/>
        <source>Project</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectManager</name>
    <message>
        <location filename="src/core/ProjectManager.cpp" line="169"/>
        <source>Open or create a project first!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/ProjectManager.cpp" line="193"/>
        <source>Could not load project %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/ProjectManager.cpp" line="198"/>
        <source>Default Project created by Traverso</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ProjectManagerDialog</name>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="25"/>
        <source>Open / Create Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="81"/>
        <source>Select Project Dir</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="114"/>
        <source>Selected Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="161"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="181"/>
        <source>Load</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="199"/>
        <source>New Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="225"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="266"/>
        <source>Engineer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="301"/>
        <source>Description</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="321"/>
        <source>Song count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="373"/>
        <source>Template</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ProjectManagerDialog.ui" line="423"/>
        <source>Create</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="125"/>
        <source>Description:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="127"/>
        <source>Created on:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="149"/>
        <source>No Project selected!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="150"/>
        <source>Select a project and click the &apos;Load&apos; button again</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="235"/>
        <source>Project does not exist! (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="229"/>
        <source>You must supply a name for the project!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="240"/>
        <source>Traverso - Question</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="197"/>
        <source>The Project &quot;%1&quot; already exists, do you want to remove it and replace it with a new one ?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="198"/>
        <source>Yes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="198"/>
        <source>No</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="219"/>
        <source>Couldn&apos;t create project (%1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="241"/>
        <source>Are you sure that you want to remove the project %1 ? It&apos;s not possible to undo it !</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="260"/>
        <source>Choose an existing or create a new Project Directory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="271"/>
        <source>Traverso - Warning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="272"/>
        <source>Unable to create Project directory! 
</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="272"/>
        <source>Please check permission for this directory: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="275"/>
        <source>Traverso - Information</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/ProjectManagerDialog.cpp" line="275"/>
        <source>Created new Project directory for you here: %1
</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="src/core/InputEngine.cpp" line="241"/>
        <source>Hold actions are not supported yet from Context Menu</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/AudioClipExternalProcessing.cpp" line="42"/>
        <source>Clip: External Processing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/Fade.cpp" line="48"/>
        <source>Fade In: range</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/Fade.cpp" line="48"/>
        <source>Fade Out: range</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/Import.cpp" line="45"/>
        <source>Import Audio File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/Import.cpp" line="59"/>
        <source>Import audio source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/Import.cpp" line="61"/>
        <source>All files (*);;Audio files (*.wav *.flac)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/MoveClip.cpp" line="40"/>
        <source>Move Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/MoveEdge.cpp" line="35"/>
        <source>Move Clip Edge</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/SplitClip.cpp" line="33"/>
        <source>Split Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/TrackPan.cpp" line="33"/>
        <source>Track Pan</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QuickDriverConfigWidget</name>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="281"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="306"/>
        <source>Apply</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="338"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="372"/>
        <source>22050</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="377"/>
        <source>32000</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="382"/>
        <source>44100</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="387"/>
        <source>48000</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="392"/>
        <source>88200</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="397"/>
        <source>96000</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="437"/>
        <source>Latency (ms)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="464"/>
        <source>Rate (Hz)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/QuickDriverConfigWidget.ui" line="485"/>
        <source>Driver</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SettingsDialog</name>
    <message>
        <location filename="src/traverso/dialogs/settings/SettingsDialog.cpp" line="74"/>
        <source>Configure - Traverso</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/SettingsDialog.cpp" line="84"/>
        <source>Behavior</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/SettingsDialog.cpp" line="90"/>
        <source>Appearance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/SettingsDialog.cpp" line="97"/>
        <source>Audio Driver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/SettingsDialog.cpp" line="104"/>
        <source>Disk I/O</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/SettingsDialog.cpp" line="111"/>
        <source>Keyboard</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Song</name>
    <message>
        <location filename="src/core/Song.cpp" line="64"/>
        <source>Untitled</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.cpp" line="67"/>
        <source>No artists name set</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.cpp" line="255"/>
        <source>Add Track</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.cpp" line="264"/>
        <source>Remove Track</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.cpp" line="571"/>
        <source>Recording to Clip(s)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.cpp" line="862"/>
        <source>Hard Disk overload detected!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.cpp" line="854"/>
        <source>Failed to fill ReadBuffer in time</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.cpp" line="863"/>
        <source>Failed to empty WriteBuffer in time</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="49"/>
        <source>Start (Play/Record)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="50"/>
        <source>Workcursor: To next ege</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="51"/>
        <source>Workcursor: To previous edge</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="52"/>
        <source>Undo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="53"/>
        <source>Redo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="54"/>
        <source>Snap: On/Off</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="55"/>
        <source>Playcursor: To workcursor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="56"/>
        <source>Solo: On/Off</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Song.h" line="57"/>
        <source>Mute: On/Off</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SongInfo</name>
    <message>
        <location filename="src/traverso/widgets/InfoWidgets.cpp" line="632"/>
        <source>Add new...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/widgets/InfoWidgets.cpp" line="634"/>
        <source>Create new Song or Track</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/widgets/InfoWidgets.cpp" line="637"/>
        <source>Record</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SongInfoWidget</name>
    <message>
        <location filename="src/traverso/ui/SongInfoWidget.ui" line="16"/>
        <source>SongInfoWidget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongInfoWidget.ui" line="58"/>
        <source>Song</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongInfoWidget.ui" line="332"/>
        <source>-</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongInfoWidget.ui" line="126"/>
        <source>Snap</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongInfoWidget.ui" line="186"/>
        <source>Gain</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongInfoWidget.ui" line="229"/>
        <source>SMPTE</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongInfoWidget.ui" line="300"/>
        <source>Zoom</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SongManagerDialog</name>
    <message>
        <location filename="src/traverso/ui/SongManagerDialog.ui" line="19"/>
        <source>Dialog</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongManagerDialog.ui" line="63"/>
        <source>Selected Song</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongManagerDialog.ui" line="93"/>
        <source>Delete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongManagerDialog.ui" line="116"/>
        <source>Rename</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongManagerDialog.ui" line="128"/>
        <source>New Song</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongManagerDialog.ui" line="150"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Bitstream Vera Sans; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Name&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongManagerDialog.ui" line="157"/>
        <source>&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;/head&gt;&lt;body style=&quot; white-space: pre-wrap; font-family:Bitstream Vera Sans; font-weight:400; font-style:normal; text-decoration:none;&quot;&gt;&lt;p style=&quot; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Artists&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SongManagerDialog.ui" line="167"/>
        <source>Create</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/project/SongManagerDialog.cpp" line="144"/>
        <source>No new Song name was supplied!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SongSelector</name>
    <message>
        <location filename="src/traverso/widgets/InfoWidgets.cpp" line="368"/>
        <source>Select Song to be displayed</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SongView</name>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="42"/>
        <source>Touch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="43"/>
        <source>Zoom: Horizontal Out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="44"/>
        <source>Zoom: Horizontal In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="45"/>
        <source>Zoom: Vertical Out</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="46"/>
        <source>Zoom: Vertical In</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="47"/>
        <source>Zoom</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="48"/>
        <source>Center View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="49"/>
        <source>Scroll: right</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="50"/>
        <source>Scroll: left</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="51"/>
        <source>Scroll: up</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="52"/>
        <source>Scroll: down</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="53"/>
        <source>Shuttle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="54"/>
        <source>Workcursor: To start</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="55"/>
        <source>Workcursor: To end</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="56"/>
        <source>Playcursor: Move</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="57"/>
        <source>Mode: Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/SongView.h" line="58"/>
        <source>Mode: Curve</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SpectralMeterConfigWidget</name>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="77"/>
        <source>SpectralMeter Configuration</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="89"/>
        <source>Frequency Range</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="101"/>
        <source>Show average spectrum</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="121"/>
        <source>Number of bands:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="141"/>
        <source>Lower dB value:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="158"/>
        <source>Upper dB value:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="188"/>
        <source> Hz</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="181"/>
        <source>Lower Limit:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="204"/>
        <source>Upper Limit:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="214"/>
        <source>Advanced FFT Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="226"/>
        <source>FFT Size:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="237"/>
        <source>256</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="242"/>
        <source>512</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="247"/>
        <source>1024</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="252"/>
        <source>2048</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="257"/>
        <source>4096</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="262"/>
        <source>8192</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="271"/>
        <source>Rectangle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="276"/>
        <source>Hanning</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="281"/>
        <source>Hamming</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="286"/>
        <source>Blackman</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="294"/>
        <source>Windowing function:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="312"/>
        <source>Advanced</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="335"/>
        <source>Apply</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SpectralMeterConfigWidget.ui" line="345"/>
        <source>&amp;Close</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SpectralMeterView</name>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.cpp" line="599"/>
        <source>Screen Capture file name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.cpp" line="637"/>
        <source>Select output format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.cpp" line="637"/>
        <source>Output format:</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.cpp" line="645"/>
        <source>Export average dB curve</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.h" line="86"/>
        <source>Settings...</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.h" line="87"/>
        <source>Toggle avarage curve</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.h" line="88"/>
        <source>Reset average curve</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.h" line="89"/>
        <source>Export avarage curve</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/SpectralMeterWidget.h" line="90"/>
        <source>Capture Screen</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SysInfoToolBar</name>
    <message>
        <location filename="src/traverso/widgets/InfoWidgets.cpp" line="699"/>
        <source>System Information</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>SystemInfoWidget</name>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="16"/>
        <source>SystemInfoWidget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="54"/>
        <source>Card Name (na)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="228"/>
        <source>image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="130"/>
        <source>buffer size (na)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="137"/>
        <source>rate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="161"/>
        <source>- GB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="168"/>
        <source>drivertype (na)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="201"/>
        <source>xruns (na)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="208"/>
        <source>latency (na)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="215"/>
        <source> - %</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/SystemInfoWidget.ui" line="235"/>
        <source>bitdepth</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ThemeConfigPage</name>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="13"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="33"/>
        <source>Theme selector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="61"/>
        <source>Path to theme files</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="142"/>
        <source>Available themes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="166"/>
        <source>Theme options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="186"/>
        <source>Adjust theme color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="208"/>
        <source>Style Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="236"/>
        <source>Select style</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/ui/ThemeConfigPage.ui" line="257"/>
        <source>Use selected style&apos;s pallet</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/dialogs/settings/Pages.cpp" line="523"/>
        <source>Select default project dir</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TimeLine</name>
    <message>
        <location filename="src/core/TimeLine.cpp" line="80"/>
        <source>Add Marker</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/TimeLine.cpp" line="97"/>
        <source>Remove Marker</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TimeLineView</name>
    <message>
        <location filename="src/traverso/songcanvas/TimeLineView.h" line="39"/>
        <source>Drag Marker</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/TimeLineView.h" line="37"/>
        <source>Add Marker</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/TimeLineView.h" line="38"/>
        <source>Remove Marker</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Track</name>
    <message>
        <location filename="src/core/Track.cpp" line="201"/>
        <source>Remove Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Track.cpp" line="213"/>
        <source>Add Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Track.cpp" line="471"/>
        <source>Silence Others</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Track.h" line="45"/>
        <source>Mute</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Track.h" line="46"/>
        <source>Record: On/Off</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Track.h" line="47"/>
        <source>Solo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/core/Track.h" line="48"/>
        <source>Silence other tracks</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TrackView</name>
    <message>
        <location filename="src/traverso/songcanvas/TrackView.cpp" line="151"/>
        <source>Set Track name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/TrackView.cpp" line="152"/>
        <source>Enter new Track name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/TrackView.h" line="36"/>
        <source>Edit properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/traverso/songcanvas/TrackView.h" line="37"/>
        <source>Add new Plugin</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>TraversoCommands</name>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="27"/>
        <source>Gain</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="28"/>
        <source>Gain: Reset</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="29"/>
        <source>Panorama</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="30"/>
        <source>Import audio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="31"/>
        <source>Copy Clip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="32"/>
        <source>New Track</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="33"/>
        <source>Remove Track</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="34"/>
        <source>Select</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="35"/>
        <source>Selection: Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/commands/plugins/TraversoCommands/TraversoCommands.h" line="36"/>
        <source>Selection: Add</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>Tsar</name>
    <message>
        <location filename="src/engine/Tsar.cpp" line="162"/>
        <source>Traverso - Malfunction!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/Tsar.cpp" line="169"/>
        <source>The Audiodriver Thread seems to be stalled/stopped, but Traverso didn&apos;t ask for it!
This effectively makes Traverso unusable, since it relies heavily on the AudioDriver Thread
To ensure proper operation, Traverso will fallback to the &apos;Null Driver&apos;.
Potential issues why this can show up are: 

* You&apos;re not running with real time privileges! Please make sure this is setup properly.

* The audio chipset isn&apos;t supported (completely), you probably have to turn off some of it&apos;s features.

For more information, see the Help file, section: 

 AudioDriver: &apos;Thread stalled error&apos;

</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/Tsar.cpp" line="176"/>
        <source>Traverso - Fatal!</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="src/engine/Tsar.cpp" line="177"/>
        <source>The Null AudioDriver stalled too, exiting application!</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
