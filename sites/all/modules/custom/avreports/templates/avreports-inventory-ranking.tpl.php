<div class="uk-grid">
  <div class="uk-width-4-6">
    <div class="uk-text-large"><?php print drupal_render($form['client_name']); ?></div>
    <?php print drupal_render($form['address']); ?>
    <?php print drupal_render($form['phone']); ?>
    <?php print drupal_render($form['agent_name']); ?>
    <div class="uk-text-large"><?php print drupal_render($form['area_name']); ?></div>
  </div>
  <div class="uk-width-2-6 uk-text-right">
    <?php print drupal_render($form['date_from']); ?>
    <?php print drupal_render($form['date_to']); ?>
    <?php print drupal_render($form['term_name']); ?>
  </div>
</div>

<?php print drupal_render($form['table']); ?>
<?php
//print drupal_render
//dpm($form);
