
<?php
//hide($form['pager']);
//print drupal_render_children($form);
?>
<!--<div class="uk-margin-top">-->
<!--  --><?php //print render($form['pager']) ?>
<!--</div>-->


<div class="uk-grid uk-grid-small">
  <div class="uk-width-4-10">
    <div class="uk-panel uk-panel-box uk-height-1-1">
      <?php print drupal_render($form['search']); ?>
    </div>
  </div>


  <div class="uk-width-medium-6-10">
    <div class="uk-panel uk-panel-box uk-height-1-1">

      <div class="uk-text-right">
        <!-- This is the container enabling the JavaScript -->
        <div class="uk-button-dropdown uk-text-left" data-uk-dropdown>
          <!-- This is the button toggling the dropdown -->
          <a class="uk-button uk-button-primary">New transaction</a>
          <!-- This is the dropdown -->
          <div class="uk-dropdown uk-dropdown-small">
            <ul class="uk-nav uk-nav-dropdown">
              <li><a href="">Purchase Order</a></li>
              <li><a href="">Receive items</a></li>
            </ul>
          </div>
        </div>
      </div>

      <h3 class="uk-panel-title">Denise Grant</h3>
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.
      Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor.

      <hr />
      <h3 class="uk-panel-title">Transactions</h3>
      <table class="uk-table uk-table-striped uk-table-condensed">
        <thead>
        <tr>
          <th>Table Heading</th>
          <th>Table Heading</th>
          <th>Table Heading</th>
        </tr>
        </thead>
        <tbody>
        <tr>
          <td>Table Data</td>
          <td>Table Data</td>
          <td>Table Data</td>
        </tr>
        <tr>
          <td>Table Data</td>
          <td>Table Data</td>
          <td>Table Data</td>
        </tr>
        <tr>
          <td>Table Data</td>
          <td>Table Data</td>
          <td>Table Data</td>
        </tr>
        </tbody>
      </table>
    </div>
  </div>
</div>
