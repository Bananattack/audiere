<?php
$current_page = 'download';
include 'page_header.inc';
?>

<h3>1.9.1</h3>

<ul>
<?php
download('audiere-1.9.1-win32.zip',   'Win32 Binary Release');
download('audiere-1.9.1.tar.gz',      'UNIX Source Release');
download('audiere-1.9.1-src.tar.bz2', 'Source Snapshot'); 
?>
</ul>

<h3>1.9.0</h3>

<ul>
<?php
download('audiere-1.9.0-win32.zip',   'Win32 Binary Release');
download('audiere-1.9.0.tar.gz',      'UNIX Source Release');
download('audiere-1.9.0-src.tar.bz2', 'Source Snapshot');
?>
</ul>

<h3>Older Files</h3>

<p>
All files are available at the <a
href="http://sourceforge.net/project/showfiles.php?group_id=32783">Audiere
project page</a>.
</p>

<?php
include 'page_footer.inc';
?>
