<div class="uk-grid uk-form-row-compact">
  <div class="uk-width-2-6">
    <div class="uk-text-bold"><?php print drupal_render($form['client_name']); ?></div>
    <?php print drupal_render($form['address']); ?>
    <?php print drupal_render($form['phone']); ?>
    <div class="uk-text-bold"><?php print drupal_render($form['agent_name']); ?></div>
  </div>
  <div class="uk-width-2-6 uk-text-center">
    <div class="uk-text-bold"><?php print drupal_render($form['area_name']); ?></div>
    <div class="uk-text-bold"><?php print drupal_render($form['principal_name']); ?></div>
  </div>
  <div class="uk-width-2-6 uk-text-right">
    <?php print drupal_render($form['avtxn.id']); ?>
    <?php print drupal_render($form['avtxn_detail.ref_txn_id']); ?>
    <?php print drupal_render($form['date_from']); ?>
    <?php print drupal_render($form['date_to']); ?>
    <?php print drupal_render($form['term_name']); ?>
  </div>
</div>

<?php print drupal_render($form['table']); ?>
<?php
//print drupal_render
//dpm($form);