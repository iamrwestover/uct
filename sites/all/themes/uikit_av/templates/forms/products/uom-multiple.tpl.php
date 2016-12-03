<?php
$uom_keys = empty($form['uoms']) ? array() : element_children($form['uoms']);
$last_uom_key = end($uom_keys);
$form['add_btn']['#attributes']['class'][] = 'av-ajax-button';
?>

<div class="uk-grid uk-grid-collapse">
  <?php foreach ($uom_keys as $uom_key): ?>
    <?php
      if ($uom_key) {
        // Hide titles except of the first UOM.
        foreach (element_children($form['uoms'][$uom_key]) as $field_name) {
          $form['uoms'][$uom_key][$field_name]['#title'] = '';
        }
        $form['uoms'][$uom_key]['delete_btn']['#attributes']['style'][] = 'margin-top: 5px;';
      }
      else {
        $form['uoms'][$uom_key]['delete_btn']['#attributes']['style'][] = 'margin-top: 30px;';
      }
      // Style delete btn.
      $form['uoms'][$uom_key]['delete_btn']['#value'] = '<i class="uk-icon-remove"></i>';
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
    ?>
    <div class="uk-width-1-1<?php //print (($last_uom_key == $uom_key) ? ' ajax-new-content' : ''); ?>">
      <div class="uk-grid uk-grid-small">
        <div class="uk-width-6-10 uk-margin-bottom"><?php print drupal_render($form['uoms'][$uom_key]['uom_id']); ?></div>
        <div class="uk-width-3-10 uk-margin-bottom"><?php print drupal_render($form['uoms'][$uom_key]['qty']); ?></div>
        <div class="uk-width-1-10 uk-margin-bottom"><?php print drupal_render($form['uoms'][$uom_key]['delete_btn']); ?></div>
      </div>
    </div>
  <?php endforeach; ?>
  <div class="uk-width-1-1 uk-margin-bottom"><?php print drupal_render($form['add_btn']); ?></div>
</div>
<?php
print drupal_render_children($form);