<?php
function entry($name, $title) {
  global $current_page;

  $url = str_replace(' ', '_', $name);

  if ($name == $current_page) {
    $begin = '<b>';
    $end   = '</b>';
  } else {
    $begin = '<a href="' . $name . '.php">';
    $end   = '</a>';
  }
  echo "<div class=\"menuitem\" title=\"$title\">$begin$name$end</div>";
}
?>

<div class="menu">

<div class="title">Audiere</div>

<br />

<?php
entry('home',          'Overview and News');
entry('features',      'Audiere's Functionality');
entry('download',      'Download the Software');
entry('documentation', 'How to Use Audiere');
entry('users',         'Projects that Use Audiere');
entry('development',   'How to Get Involved');
entry('lists',         'Mailing List Information');
entry('links',         'Links to External Sites');
entry('contact',       'How to Reach the Developers');
?>

<br />

<div class="images">
  <a href="http://validator.w3.org/check/referer">
    <img src="http://www.w3.org/Icons/valid-xhtml10"
         alt="Valid XHTML 1.0!" />
  </a>
  <br />
  <a href="http://jigsaw.w3.org/css-validator/">
    <img src="http://jigsaw.w3.org/css-validator/images/vcss"
         alt="Valid CSS!" />
  </a>
  <br />
  <a href="http://sourceforge.net/">
    <img src="http://sourceforge.net/sflogo.php?group_id=32783&amp;type=1"
         alt="SourceForge Logo" />
  </a>
</div>

</div>
