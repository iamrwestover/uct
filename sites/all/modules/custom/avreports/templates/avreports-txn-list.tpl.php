<?php
  $mid_class = isset($form['client_name']) ? 'uk-width-1-3 uk-text-center' : 'uk-width-2-3';
?>
<div class="uk-grid uk-form-row-compact">
  <?php if (isset($form['client_name'])): ?>
    <div class="uk-width-1-3">
      <?php print drupal_render($form['client_name']); ?>
    </div>
  <?php endif; ?>
  <div class="<?php print $mid_class; ?>">
    <?php if (isset($form['agent_name'])): ?>
      <?php print drupal_render($form['agent_name']); ?>
    <?php endif; ?>
    <?php if (isset($form['area_name'])): ?>
      <?php print drupal_render($form['area_name']); ?>
    <?php endif; ?>
    <?php if (isset($form['principal_name'])): ?>
      <?php print drupal_render($form['principal_name']); ?>
    <?php endif; ?>
  </div>
  <div class="uk-width-1-3 uk-text-right">
    <?php if (isset($form['simplified_status'])): ?>
      <?php print drupal_render($form['simplified_status']); ?>
    <?php endif; ?>
    <?php print drupal_render($form['date_from']); ?>
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
