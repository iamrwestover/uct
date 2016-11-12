<?php
// Remove fieldset titles.
unset($form['info']['#title']);

// Wrap buttons.
$form['buttons']['#prefix'] = '<div class="uk-margin-top">';
$form['buttons']['#suffix'] = '</div>';

$form['info']['initial_qty_date']['#icon_key'] = 'calendar';
$form['info']['initial_qty_date']['#attributes']['data-uk-datepicker'] = "{format:'MMM. DD, YYYY'}";
?>

<div class="uk-grid uk-grid-large">
  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-2-3 uk-margin-small-bottom"><?php print drupal_render($form['info']['title']); ?></div>
      <div class="uk-width-1-3 uk-margin-small-bottom"><?php print drupal_render($form['info']['code']); ?></div>

      <div class="uk-width-2-3 uk-margin-small-bottom"><?php print drupal_render($form['info']['category_id']); ?></div>
      <div class="uk-width-1-3 uk-margin-small-bottom"><?php print drupal_render($form['info']['uom_id']); ?></div>

      <div class="uk-width-1-1 uk-margin-small-bottom"><?php print drupal_render($form['info']['description']); ?></div>
    </div>
  </div>


  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-1-4 uk-margin-small-bottom"><?php print drupal_render($form['info']['cost']); ?></div>
      <div class="uk-width-1-4 uk-margin-small-bottom"><?php print drupal_render($form['info']['price']); ?></div>
      <div class="uk-width-1-4 uk-margin-small-bottom"><?php print drupal_render($form['info']['rop']); ?></div>
      <div class="uk-width-1-4 uk-margin-small-bottom"><?php print drupal_render($form['info']['eoq']); ?></div>

      <div class="uk-width-1-2 uk-margin-small-bottom"><?php print drupal_render($form['info']['initial_qty']); ?></div>
      <div class="uk-width-1-2 uk-margin-small-bottom"><?php print drupal_render($form['info']['initial_qty_date']); ?></div>
    </div>
  </div>

</div>



<?php
$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
$form['buttons']['cancel']['#markup'] = l('Cancel', 'av/products', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);