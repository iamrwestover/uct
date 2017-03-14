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
?>
<div class="uk-grid uk-grid-small uk-margin-top">
  <?php if ($transaction_id_html): ?>
    <div class="uk-width-1-10">
      <?php print $transaction_id_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($search_text_html): ?>
    <div class="uk-width-3-10">
      <?php print $search_text_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($transaction_type_html): ?>
    <div class="uk-width-2-10">
      <?php print $transaction_type_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($status_html): ?>
    <div class="uk-width-1-10">
      <?php print $status_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($client_html): ?>
    <div class="uk-width-3-10 uk-margin-small-bottom">
      <?php print $client_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($transaction_date): ?>
    <div class="uk-width-2-10">
      <?php print $transaction_date; ?>
    </div>
  <?php endif; ?>

  <?php if ($agent_html): ?>
    <div class="uk-width-2-10">
      <?php print $agent_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($principal_html): ?>
    <div class="uk-width-3-10">
      <?php print $principal_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($area_html): ?>
    <div class="uk-width-3-10">
      <?php print $area_html; ?>
    </div>
  <?php endif; ?>

  <?php if ($date_from): ?>
    <div class="uk-width-2-10">
      <?php print $date_from; ?>
    </div>
  <?php endif; ?>
  <?php if ($date_to): ?>
    <div class="uk-width-2-10">
      <?php print $date_to; ?>
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
