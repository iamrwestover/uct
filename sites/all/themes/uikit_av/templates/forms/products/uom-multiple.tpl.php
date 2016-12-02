<?php
$uom_keys = empty($form['uoms']) ? array() : element_children($form['uoms']);
$last_uom_key = end($uom_keys);
?>

<div class="uk-grid uk-grid-collapse">
  <?php foreach ($uom_keys as $uom_key): ?>
    <?php
      if ($uom_key) {
        // Hide titles except of the first UOM.
        foreach (element_children($form['uoms'][$uom_key]) as $field_name) {
          $form['uoms'][$uom_key][$field_name]['#title'] = '';
        }
      }
    ?>
    <div class="uk-width-1-1<?php print ($last_uom_key == $uom_key) ? ' ajax-nsew-content' : ''; ?>">
      <div class="uk-grid uk-grid-small">
        <div class="uk-width-1-2 uk-margin-bottom"><?php print drupal_render($form['uoms'][$uom_key]['uom_id']); ?></div>
        <div class="uk-width-1-2 uk-margin-bottom"><?php print drupal_render($form['uoms'][$uom_key]['repeater_test']); ?></div>
      </div>
    </div>
  <?php endforeach; ?>
  <div class="uk-width-1-1 uk-margin-bottom"><?php print drupal_render($form['add_more']); ?></div>
</div>
<?php
print drupal_render_children($form);