<?php
$current_page = 'home';
include 'page_header.inc';
?>

<h2>Overview</h2>

<p>
Audiere is a high-level audio API.  It can understand
<a href="http://vorbis.com/">Ogg Vorbis</a>, uncompressed WAV,
MOD, S3M, XM, IT.  It supports DirectSound in Windows and OSS on
UNIX.
</p>

<p>
Audiere is <a href="http://opensource.org/">open source</a> and licensed under
the <a href="http://opensource.org/licenses/lgpl-license.html">LGPL</a>.
Basically, if you make changes to the library, you have to release the source
to your changes.
</p>

<p>
Audiere is relatively portable.  It is tested on Windows and Linux-i386.
Unfortunately, Audiere depends on MikMod, which has all sorts of endian issues,
and most certainly does not work on big-endian architectures.
</p>

<h2>News</h2>

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
