<?php
$current_page = 'contact';
include 'page_header.inc';

function developer($name, $url, $desc) {
  echo '<li>';
  echo "<a href=\"$url\">$name</a> - $desc";
  echo '</li>';
}

echo '<ul>';

developer('Chad Austin',
          'http://aegisknight.org/',
          'Lead Developer');
developer('Jacky Chong',
          'http://www.livejournal.com/users/darklich/',
          'Contributor');
developer('Theo Reed',
          'http://surreality.us:8080/~rizen/',
          'Contributor');
developer('Ben Scott',
          'http://www.vrac.iastate.edu/~bscott/',
          'Contributor');

echo '</ul>';
?>

<p>
Please take a look at the <a href="lists.php">mailing lists</a> page for
information on how to contact the developers.
</p>

<?php
include 'page_footer.inc';
?>
