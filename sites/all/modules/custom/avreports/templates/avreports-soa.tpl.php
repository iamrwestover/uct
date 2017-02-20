<div class="uk-grid">
  <div class="uk-width-5-6">
    <strong><?php print drupal_render($form['client_name']); ?></strong>
    <?php print drupal_render($form['address']); ?>
    <?php print drupal_render($form['phone']); ?>
  </div>
  <div class="uk-width-1-6">
    <?php print drupal_render($form['date_to']); ?>
    <?php print drupal_render($form['term_name']); ?>
  </div>
</div>

<?php print drupal_render($form['table']); ?>
<?php print drupal_render($form['balance_ages']); ?>

<div class="uk-grid uk-margin-top">
  <div class="uk-width-1-2">
    <?php print drupal_render($form['prepared_by']); ?>
  </div>
  <div class="uk-width-1-2 uk-text-right">
    <?php print drupal_render($form['received_by']); ?>
  </div>
</div>
<?php
//print drupal_render
//dpm($form);
