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
          'Developer and Project Manager');
developer('Jacky Chong',
          'http://www.livejournal.com/users/darklich/',
          'Developer');
developer('Theo Reed',
          'http://surreality.us:8080/~rizen/',
          'Developer');
developer('Ben Scott',
          'http://www.vrac.iastate.edu/~bscott/',
          'Developer');

echo '</ul>';
?>

<p>
Please take a look at the <a href="lists.php">mailing lists</a> page for
information on how to contact the developers.
</p>

<?php
include 'page_footer.inc';
?>
