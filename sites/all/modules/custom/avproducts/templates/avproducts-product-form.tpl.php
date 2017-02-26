<?php
//
$form['info']['#access'] = FALSE;
$form['uom']['#access'] = FALSE;



// Misc.
$form['info']['cost']['#attributes']['class'][] = 'uk-text-right';
$form['info']['price']['#attributes']['class'][] = 'uk-text-right';
$form['info']['qty']['#attributes']['class'][] = 'uk-text-right';

$qty_html = drupal_render($form['info']['qty']);
$initial_qty_html = drupal_render($form['info']['initial_qty']);
$initial_qty_date_html = drupal_render($form['info']['initial_qty_date']);
?>

<div class="uk-grid uk-grid-large">
  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">
      <div class="uk-width-2-3 uk-margin-bottom"><?php print drupal_render($form['info']['title']); ?></div>
      <div class="uk-width-1-3 uk-margin-bottom"><?php print drupal_render($form['info']['code']); ?></div>

      <div class="uk-width-2-3 uk-margin-bottom"><?php print drupal_render($form['info']['vendor_id']); ?></div>
      <div class="uk-width-1-3 uk-margin-bottom"><?php print drupal_render($form['info']['principal_id']); ?></div>

      <div class="uk-width-1-3 uk-margin-bottom"><?php print drupal_render($form['info']['category_id']); ?></div>
      <!--<div class="uk-width-1-3 uk-margin-bottom">--><?php //print drupal_render($form['info']['account_type_id']); ?><!--</div>-->
      <div class="uk-width-1-3 uk-margin-bottom"><?php print drupal_render($form['info']['shelf_id']); ?></div>

      <div class="uk-width-1-3 uk-margin-bottom"><?php print drupal_render($form['info']['expiry_date']); ?></div>
      <div class="uk-width-1-3 uk-margin-bottom"><?php print drupal_render($form['info']['rop']); ?></div>
      <div class="uk-width-1-3 uk-margin-bottom"><?php print drupal_render($form['info']['eoq']); ?></div>
      <div class="uk-width-1-3 uk-margin-bottom"><?php print drupal_render($form['info']['discount']); ?></div>

      <!--<div class="uk-width-1-1">--><?php //print drupal_render($form['info']['description']); ?><!--</div>-->
    </div>
  </div>


  <div class="uk-width-1-2">
    <div class="uk-grid uk-grid-small">

      <?php if ($initial_qty_date_html): ?>
        <div class="uk-width-7-10 uk-margin-bottom"><?php print $initial_qty_date_html; ?></div>
      <?php endif; ?>
      <div class="uk-width-3-10 uk-margin-bottom"><?php print drupal_render($form['info']['uom_id']); ?></div>
      <?php if ($initial_qty_html): ?>
        <div class="uk-width-2-10 uk-margin-bottom"><?php print $initial_qty_html; ?></div>
      <?php endif; ?>
      <?php if ($qty_html): ?>
        <div class="uk-width-2-10 uk-margin-bottom"><?php print $qty_html; ?></div>
      <?php endif; ?>

      <div class="uk-width-2-10 uk-margin-bottom"><?php print drupal_render($form['info']['cost']); ?></div>
      <div class="uk-width-2-10 uk-margin-bottom"><?php print drupal_render($form['info']['price']); ?></div>

      <div class="uk-width-1-1 uk-margin-bottom"><?php print drupal_render($form['uom']['uom_group']); ?></div>
    </div>
  </div>

</div>



<?php
//$form['buttons']['submit']['#attributes']['class'][] = 'uk-button-primary';
$close_btn_label = empty($form['id']['#value']) ? 'Cancel' : 'Close';
$form['buttons']['cancel']['#markup'] = l($close_btn_label, 'av/products', array('attributes' => array('class' => array('uk-button'))));
print drupal_render_children($form);