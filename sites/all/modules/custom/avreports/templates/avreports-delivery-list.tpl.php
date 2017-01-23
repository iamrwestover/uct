<div class="uk-grid uk-margin-top">
  <div class="uk-width-1-2">
    <?php print drupal_render($form['agent']); ?>
  </div>
  <div class="uk-width-1-2 uk-text-right">
    <?php print drupal_render($form['transaction_date']); ?>
  </div>
</div>

<?php print drupal_render($form['table']); ?>

<div class="uk-grid uk-margin-top">
  <div class="uk-width-1-2">
    <?php print drupal_render($form['prepared_by']); ?>
  </div>
  <div class="uk-width-1-2 uk-text-right">
    <?php print drupal_render($form['delivered_by']); ?>
  </div>
</div>
<?php
//print drupal_render
//dpm($form);
