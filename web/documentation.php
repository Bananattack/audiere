<?php
$current_page = 'documentation';
include 'page_header.inc';
?>

<h2>API Reference</h2>

<p>
We use <a href="http://doxygen.org/">Doxygen</a> to generate API
documentation from source code and specially-formatted comments.
This includes documentation for both external interfaces and interfaces
not exported by the library.  Therefore, it is probably more useful
to developers than users of the library, but don't let that stop you
from looking at it.
</p>

<ul>
<li><a href="audiere-1.9.3-doxygen/">API Reference (Web)</a></li>
<?php
download('audiere-1.9.3-doxygen.chm',     'API Reference (HTMLHelp format)');
download('audiere-1.9.3-doxygen.zip',     'API Reference (.zip format)');
download('audiere-1.9.3-doxygen.tar.bz2', 'API Reference (.tar.bz2 format)');
?>
</ul>

<h2>Frequently Asked Questions</h2>

<p>
You can view the FAQ directly from CVS
<a href="http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/*checkout*/audiere/audiere/doc/faq.txt">here</a>.
</p>

<p>
More documentation is included in the <a href="download.php">downloads</a>.
</p>

<?php
include 'page_footer.inc';
?>
