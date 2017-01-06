<?php
$uom_keys = empty($form['uoms']) ? array() : element_children($form['uoms']);
$last_uom_key = end($uom_keys);
$form['add_btn']['#attributes']['class'][] = 'av-ajax-button';
?>

<div class="uk-grid uk-grid-collapse">
  <?php foreach ($uom_keys as $uom_key): ?>
    <?php
      if ($uom_keys[0] != $uom_key) {
      //if (1) {
        // Hide titles except of the first UOM.
        foreach (element_children($form['uoms'][$uom_key]) as $field_name) {
          $form['uoms'][$uom_key][$field_name]['#title_display'] = 'none';
        }
        $form['uoms'][$uom_key]['delete_btn']['#attributes']['style'][] = 'margin-top: 5px;';
        $form['uoms'][$uom_key]['auto_compute_price']['#attributes']['style'][] = 'margin-top: 5px;';
      }
      else {
        $form['uoms'][$uom_key]['delete_btn']['#attributes']['style'][] = 'margin-top: 30px;';
        $form['uoms'][$uom_key]['auto_compute_price']['#attributes']['style'][] = 'margin-top: 33px;';
      }
      // Style delete btn.
      $form['uoms'][$uom_key]['delete_btn']['#button_label'] = '<i class="uk-icon-remove"></i>';
      $form['uoms'][$uom_key]['delete_btn']['#attributes']['class'][] = 'av-ajax-button';
      $form['uoms'][$uom_key]['delete_btn']['#attributes']['class'][] = 'av-ajax-button-icon-only';
      $form['uoms'][$uom_key]['delete_btn']['#attributes']['class'][] = 'uk-button-danger';
      $form['uoms'][$uom_key]['delete_btn']['#attributes']['class'][] = 'uk-button-mini';
      // Style qty field.
      $form['uoms'][$uom_key]['qty']['#attributes']['class'][] = 'uk-text-right';
      $form['uoms'][$uom_key]['qty']['#attributes']['data-uk-tooltip'] = "{pos:'bottom'}";
      $form['uoms'][$uom_key]['qty']['#attributes']['title'] = $form['uoms'][$uom_key]['qty']['#description'];
      unset($form['uoms'][$uom_key]['qty']['#description']);
      $form['uoms'][$uom_key]['qty']['#required'] = FALSE;

      // Auto-compute toggle.
      //$form['uoms'][$uom_key]['auto_compute_price']['#title'] = '';
      //$form['uoms'][$uom_key]['auto_compute_price']['#attributes']['data-uk-tooltip'] = "{pos:'top'}";
      //$form['uoms'][$uom_key]['auto_compute_price']['#attributes']['title'] = 'Auto-compute price';

      // Miscellaneous.
      $form['uoms'][$uom_key]['cost']['#attributes']['class'][] = 'uk-text-right';
      $form['uoms'][$uom_key]['price']['#attributes']['class'][] = 'uk-text-right';
    ?>
    <div class="uk-width-1-1<?php //print (($last_uom_key == $uom_key) ? ' ajax-new-content' : ''); ?>">
      <div class="uk-grid uk-grid-small">
        <div class="uk-width-3-10 uk-margin-small-bottom"><?php print drupal_render($form['uoms'][$uom_key]['uom_id']); ?></div>
        <div class="uk-width-2-10 uk-margin-small-bottom"><?php print drupal_render($form['uoms'][$uom_key]['qty']); ?></div>
        <div class="uk-width-2-10 uk-margin-small-bottom"><?php print drupal_render($form['uoms'][$uom_key]['cost']); ?></div>
        <div class="uk-width-2-10 uk-margin-small-bottom"><?php print drupal_render($form['uoms'][$uom_key]['price']); ?></div>
        <div class="uk-width-1-10 uk-margin-small-bottom uk-text-center"><?php print drupal_render($form['uoms'][$uom_key]['delete_btn']); ?></div>
        <!--<div class="uk-width-1-10 uk-margin-bottom">-->
        <!--  <div class="uk-grid uk-grid-small">-->
        <!--    <div class="uk-width-1-2 uk-margin-bottom">--><?php //print drupal_render($form['uoms'][$uom_key]['auto_compute_price']); ?><!--</div>-->
        <!--    <div class="uk-width-1-2 uk-margin-bottom uk-text-center">--><?php //print drupal_render($form['uoms'][$uom_key]['delete_btn']); ?><!--</div>-->
        <!--  </div>-->
        <!--</div>-->

      </div>
    </div>
  <?php endforeach; ?>
  <div class="uk-width-1-1 uk-margin-bottom"><?php print drupal_render($form['add_btn']); ?></div>
</div>
<?php
print drupal_render_children($form);