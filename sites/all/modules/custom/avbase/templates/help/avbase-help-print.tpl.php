<?php
//var_dump(get_defined_vars());
?>
<h2 id="help-print"><?php print $help_title; ?></h2>
<p>
  To print on dot matrix printers, you should select "Default" as margin, and <em>UCT 8.27" x 11"</em> as paper size.
  If <em>UCT 8.27" x 11"</em> is not available, you need to create it first.
  To do this..
</p>
<ol>
  <li>Go to the computer where your printer is installed. Open your printer's properties window.
    <ul>
      <li>If you don't know how to find your specific printer's properties window, open chrome and try to print a page.</li>
      <li>Click "Print using system dialog" on the print window.</li>
    </ul>
  </li>
  <li>Under "Select Printer" options, make sure your dot matrix printer is selected. Then click "Preferences".</li>
  <li>Click the "User Defined Paper" Tab</li>
  <li>Enter any name for your new paper size, we recommend using the name <em>UCT 8.27" x 11"</em></li>
  <li>Choose "inch" on the Unit option.</li>
  <li>Input <em>8.27</em> on width, and <em>11</em> on length.</li>
  <li>Click OK to save your changes.</li>
  <li>Click Apply.</li>
  <!--<li>Then click Cancel to close the system print dialog.</li>-->
</ol>
<p>Try printing again, the option <em>UCT 8.27" x 11"</em> should now be available on the Paper size dropdown.</p>
