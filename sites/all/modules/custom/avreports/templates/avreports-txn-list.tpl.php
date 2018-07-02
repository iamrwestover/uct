<?php
  $mid_class = isset($form['client_name']) ? 'uk-width-1-3 uk-text-center' : 'uk-width-2-3';
?>
<div class="uk-grid uk-form-row-compact">
  <?php if (isset($form['submit'])): ?>
    <div class="uk-width-1-1">
      <?php print drupal_render($form['submit']); ?>
    </div>
  <?php endif; ?>
  <?php if (isset($form['client_name'])): ?>
    <div class="uk-width-1-3">
      <?php print drupal_render($form['client_name']); ?>
    </div>
  <?php endif; ?>
  <div class="<?php print $mid_class; ?>">
    <?php if (isset($form['product_name'])): ?>
      <?php print drupal_render($form['product_name']); ?>
    <?php endif; ?>
    <?php if (isset($form['agent_name'])): ?>
      <?php print drupal_render($form['agent_name']); ?>
    <?php endif; ?>
    <?php if (isset($form['area_name'])): ?>
      <?php print drupal_render($form['area_name']); ?>
    <?php endif; ?>
    <?php if (isset($form['principal_name'])): ?>
      <?php print drupal_render($form['principal_name']); ?>
    <?php endif; ?>
    <?php if (isset($form['return_type'])): ?>
      <?php print drupal_render($form['return_type']); ?>
    <?php endif; ?>
    <?php if (isset($form['avtxn.reference_id'])): ?>
      <?php print drupal_render($form['avtxn.reference_id']); ?>
    <?php endif; ?>
    <?php if (isset($form['mid_info'])): ?>
      <?php print drupal_render($form['mid_info']); ?>
    <?php endif; ?>
  </div>
  <div class="uk-width-1-3 uk-text-right">
    <?php if (isset($form['simplified_status'])): ?>
      <?php print drupal_render($form['simplified_status']); ?>
    <?php endif; ?>
    <?php print drupal_render($form['date_from']); ?>
    <?php if (isset($form['payment_date_to'])): ?>
      <?php print drupal_render($form['payment_date_to']); ?>
    <?php endif; ?>
    <?php if (isset($form['avclients.category_id'])): ?>
      <?php print drupal_render($form['avclients.category_id']); ?>
    <?php endif; ?>
    <?php if (isset($form['avcat_prodcat.id'])): ?>
      <?php print drupal_render($form['avcat_prodcat.id']); ?>
    <?php endif; ?>
    <?php if (isset($form['other_info'])): ?>
      <?php print drupal_render($form['other_info']); ?>
    <?php endif; ?>
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
