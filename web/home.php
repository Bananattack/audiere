<?php
$current_page = 'home';
include 'page_header.inc';
?>

<h2>Overview</h2>

<p>
Audiere is a high-level audio API.  It can understand
<a href="http://vorbis.com/">Ogg Vorbis</a>, uncompressed WAV,
MOD, S3M, XM, IT.  It supports DirectSound in Windows and OSS on
Linux and Cygwin, and SGI AL on IRIX.
</p>

<p>
Audiere is <a href="http://opensource.org/">open source</a> and licensed under
the <a href="http://opensource.org/licenses/lgpl-license.html">LGPL</a>.
Basically, if you make changes to the library, you have to release the source
to your changes.
</p>

<p>
Audiere is relatively portable.  It is tested on Windows, Linux-i386, Cygwin,
and IRIX.  Most of Audiere is endianness-independent, so I expect it would
work with few modifications on other architectures.
</p>

<h2>News</h2>

<h3>2002.09.07 - Audiere 1.9.0 Released</h3>

<p>
Finally, it's here!  This is a major release, designed to get the new API
tested before 2.0 is released.  I would say 1.9.0 should be considered beta,
but I feel it is so much better than the last 'stable' release (1.0.4) that
there is no reason not to upgrade.
</p>

<p>
The following is planned for 2.0: 3D spatialization, FLAC support,
MP3 support (again, this time using an LGPL library), supported file
type enumeration, pitch bending and other DSP effects, AIFF support,
and ADPCM compressed WAV support.
</p>

<p>
Here is what changed since 1.0.4:
</p>

<ul>
<li>completely new API, defined in C++</li>
<li>made Audiere objects reference counted</li>
<li>support for seeking within WAV and Ogg Vorbis files</li>
<li>support for preloading sounds instead of always streaming from disk</li>
<li>updated Python, XPCOM, and Java bindings</li>
<li>upgraded to Ogg Vorbis 1.0</li>
<li>major performance improvement in DSOutputStream::isPlaying</li>
<li>Ogg Vorbis decoder now works properly on big-endian architectures</li>
<li>WAV reader works properly on big-endian architectures</li>
<li>general support for big-endian architectures</li>
<li>SGI AL (Audio Library) output support</li>
<li>WinMM output support</li>
<li>new SampleBuffer object for loading a sound once and playing it multiple times</li>
<li>SCons build system for MIPSPro on IRIX</li>
<li>interfaces now use __stdcall on Windows so they are more compliant with COM</li>
<li>completely new wxPlayer</li>
<li>fixed bug on Linux where, the more streams were playing, the lower the overall volume would be</li>
<li>implemented the REAL fix for the DirectSound stream-repeating bug</li>
<li>support for low-pass filters on .it files (enabled DMODE_RESONANCE in mikmod)</li>
<li>readded stereo panning support</li>
<li>switched volume to normalized floats [0, 1] instead of [0, 255]</li>
<li>added a tone generator</li>
</ul>

<h3>2002.06.10 - Audiere 1.0.4 Released</h3>

<p>
Audiere 1.0.4 is out.  This is a relatively minor release, but I highly
recommend that users of 1.0.3 upgrade to this version.  One thing to keep
in mind is that I have removed support for MP3 playback.  See the
<a href="http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/*checkout*/audiere/audiere/doc/faq.txt">FAQ</a>
for the rationale behind this decision.
</p>

<p>
With the release of 1.0.4 came a "new" web design (copied from
<a href="http://corona.sf.net/">Corona</a>, which was, in turn,
copied from <a href="http://www.scons.org/">SCons</a>).  Complaints?
Praise?  Send <a href="mailto:aegis@aegisknight.org">me</a> e-mail.
</p>

<p>
Here is what changed since 1.0.3.
</p>

<ul>
<li>fixed awful sound repeating bug</li>
<li>removed MP3 support</li>
<li>switched library completely to LGPL</li>
<li>compiles on Debian</li>
<li>compiles with gcc 3</li>
<li>compiles on IRIX (no mikmod or audio output support yet)</li>
<li>correct timing in null output driver</li>
<li>major source tree restructuring</li>
<li>--enable-debug configuration option</li>
<li>added Doxygen support</li>
<li>fixed noisy output in Linux</li>
<li>removed Acoustique, moved decoder architecture into Audiere itself</li>
<li>fixed isPlaying() in OSS output driver</li>
<li>runs on NT4 again</li>
<li>removed DLL output driver</li>
<li>removed null output driver from default list (it must be explicit)</li>
<li>fixed complete hang on context creation in Win9x</li>
<li>support building without DX8</li>
<li>several internal mikmod updates</li>
</ul>

<?php
include 'page_footer.inc';
?>
