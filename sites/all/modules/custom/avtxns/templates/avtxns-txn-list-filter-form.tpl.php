<?php
$search_text_html = drupal_render($form['search_text']);
$client_html = drupal_render($form['avclients.display_name']);
$agent_html = drupal_render($form['avcat_agent.title']);
$transaction_date = drupal_render($form['avtxn.transaction_date']);
$date_to = drupal_render($form['date_to']);
$date_from = drupal_render($form['date_from']);
$transaction_id_html = drupal_render($form['avtxn.id']);
$transaction_type_html = drupal_render($form['avtxn.transaction_type']);
$area_html = drupal_render($form['avcat_area.title']);
$principal_html = drupal_render($form['avcat_principal.title']);
$status_html = drupal_render($form['avtxn.status']);
$reset_btn_html = drupal_render($form['buttons']['reset_button']);
$table_group_by_html = drupal_render($form['settings']['table_group_by']);
$table_columns_html = drupal_render($form['settings']['table_columns']);

?>
<div class="uk-grid uk-grid-small">
  <?php if ($transaction_id_html): ?>
    <div class="uk-width-1-10">
      <?php print $transaction_id_html; ?>
    </div>
  <?php endif; ?>

  <?php if (isset($form['avtxn_detail.ref_txn_id'])): ?>
    <div class="uk-width-1-10">
      <?php print drupal_render($form['avtxn_detail.ref_txn_id']); ?>
    </div>
  <?php endif; ?>

  <?php if ($search_text_html): ?>
    <div class="uk-width-3-10">
      <?php print $search_text_html; ?>
    </div>
  <?php endif; ?>

  <?php if (isset($form['avprod.title'])): ?>
    <div class="uk-width-3-10">
      <?php print drupal_render($form['avprod.title']); ?>
    </div>
  <?php endif; ?>

  <?php if (isset($form['avcat_prodcat.id'])): ?>
    <div class="uk-width-1-10">
      <?php print drupal_render($form['avcat_prodcat.id']); ?>
    </div>
  <?php endif; ?>



  <?php if ($status_html): ?>
    <div class="uk-width-1-10">
      <?php print $status_html; ?>
    </div>
  <?php endif; ?>

  <?php if (isset($form['settings']['date_auto'])): ?>
    <div class="uk-width-2-10">
      <?php print drupal_render($form['settings']['date_auto']); ?>
    </div>
  <?php endif; ?>
  <?php if ($date_from): ?>
    <div class="uk-width-2-10 uk-margin-small-bottom">
      <?php print $date_from; ?>
    </div>
  <?php endif; ?>
  <?php if ($date_to): ?>
    <div class="uk-width-2-10">
      <?php print $date_to; ?>
    </div>
  <?php endif; ?>

  <?php if ($transaction_type_html): ?>
    <div class="uk-width-1-10">
      <?php print $transaction_type_html; ?>
    </div>
  <?php endif; ?>


  <?php if ($client_html): ?>
    <div class="uk-width-3-10 uk-margin-small-bottom">
      <?php print $client_html; ?>
    </div>
  <?php endif; ?>

  <?php if (isset($form['avclients.category_id'])): ?>
    <div class="uk-width-1-10">
      <?php print drupal_render($form['avclients.category_id']); ?>
    </div>
  <?php endif; ?>


  <?php if ($agent_html): ?>
    <div class="uk-width-2-10">
      <?php print $agent_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($area_html): ?>
    <div class="uk-width-2-10">
      <?php print $area_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($principal_html): ?>
    <div class="uk-width-2-10">
      <?php print $principal_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($transaction_date): ?>
    <div class="uk-width-1-10">
      <?php print $transaction_date; ?>
    </div>
  <?php endif; ?>



  <?php if (isset($form['settings']['principal_name'])): ?>
    <div class="uk-width-2-10">
      <?php print drupal_render($form['settings']['principal_name']); ?>
    </div>
  <?php endif; ?>



  <?php if (isset($form['simplified_status'])): ?>
    <div class="uk-width-2-10">
      <?php print drupal_render($form['simplified_status']); ?>
    </div>
  <?php endif; ?>
  <?php if (isset($form['overdue'])): ?>
    <div class="uk-width-1-10">
      <?php print drupal_render($form['overdue']); ?>
    </div>
  <?php endif; ?>



  <?php if ($table_group_by_html): ?>
    <div class="uk-width-1-10">
      <?php print $table_group_by_html; ?>
    </div>
  <?php endif; ?>
  <?php if ($table_columns_html): ?>
    <div class="uk-width-1-10">
      <?php print $table_columns_html; ?>
    </div>
  <?php endif; ?>

  <?php if (isset($form['settings']['all_products'])): ?>
    <div class="uk-width-2-10">
      <?php print drupal_render($form['settings']['all_products']); ?>
    </div>
  <?php endif; ?>
  <?php if (isset($form['settings']['stock_status'])): ?>
    <div class="uk-width-1-10">
      <?php print drupal_render($form['settings']['stock_status']); ?>
    </div>
  <?php endif; ?>
  <?php if (isset($form['settings']['preferred_vendor'])): ?>
    <div class="uk-width-3-10">
      <?php print drupal_render($form['settings']['preferred_vendor']); ?>
    </div>
  <?php endif; ?>

  <?php if (isset($form['settings']['selectable'])): ?>
    <div class="uk-width-1-10">
      <?php print drupal_render($form['settings']['selectable']); ?>
    </div>
  <?php endif; ?>

  <?php if (isset($form['client_status'])): ?>
    <div class="uk-width-2-10">
      <?php print drupal_render($form['client_status']); ?>
    </div>
  <?php endif; ?>
  <?php if (isset($form['product_status'])): ?>
    <div class="uk-width-2-10">
      <?php print drupal_render($form['product_status']); ?>
    </div>
  <?php endif; ?>
  <?php if (isset($form['return_type'])): ?>
    <div class="uk-width-1-10">
      <?php print drupal_render($form['return_type']); ?>
    </div>
  <?php endif; ?>


  <?php if (isset($form['avtxn.reference_id'])): ?>
    <div class="uk-width-1-10">
      <?php print drupal_render($form['avtxn.reference_id']); ?>
    </div>
  <?php endif; ?>

  <?php if ($reset_btn_html): ?>
    <div class="uk-width-1-10">
      <?php print $reset_btn_html; ?>
    </div>
  <?php endif; ?>
</div>
<?php print drupal_render($form['search_button']); ?>
<?php
print drupal_render_children($form);
