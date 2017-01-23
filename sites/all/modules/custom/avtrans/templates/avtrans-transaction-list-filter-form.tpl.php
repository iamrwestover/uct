<?php
$search_text_html = drupal_render($form['search_text']);
$agent_html = drupal_render($form['avcat_agent.title']);
$transaction_id_html = drupal_render($form['avtrans.id']);
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

  <div class="uk-width-2-10">
    <?php print drupal_render($form['avtrans.transaction_date']); ?>
  </div>

  <?php if ($agent_html): ?>
    <div class="uk-width-3-10">
      <?php print $agent_html; ?>
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
