<?php
$view_mode = !empty($form['#av_view_mode']);
//foreach (element_children($form) as $key) {
//  $form[$key]['#title_display'] = 'none';
//}

//$form['product_title']['#attributes']['placeholder'] = 'product code or name';

//$form['uom_title']['#attributes']['class'][] = 'uk-text-center';
//$form['qty']['#attributes']['class'][] = 'uk-text-right';
//$form['cost']['#attributes']['class'][] = 'uk-text-right';
//$form['discount']['#attributes']['class'][] = 'uk-text-right';
//$form['total']['#attributes']['class'][] = 'uk-text-right';

// Style delete btn.
//$form['prod_delete_btn']['#button_label'] = '<i class="uk-icon-remove"></i>';
//$form['prod_delete_btn']['#attributes']['class'][] = 'av-ajax-trigger';
//$form['prod_delete_btn']['#attributes']['class'][] = 'av-ajax-trigger-icon-only';
//$form['prod_delete_btn']['#attributes']['class'][] = 'uk-button-danger';
//$form['prod_delete_btn']['#attributes']['class'][] = 'uk-button-mini';
//$form['prod_delete_btn']['#attributes']['class'][] = 'uk-text-muted';

?>

<div class="uk-grid uk-grid-collapse no-page-break-inside" style="position: relative;">
  <?php print drupal_render($form['badge']); ?>
  <div class="uk-width-6-10">

    <div class="uk-grid uk-grid-collapse">
      <div class="uk-width-1-10" style="width: 10%;">
        <div class="av-nestable-cell uk-text-center">
          <?php print drupal_render($form['qty']); ?>
        </div>
      </div>

      <div class="uk-width-1-10" style="width: 12%;">
        <div class="av-nestable-cell uk-text-left">
          <?php print drupal_render($form['uom_title']); ?>
          <?php print drupal_render($form['qty_per_uom']); ?>
        </div>
      </div>

      <div class="uk-width-8-10" style="width: 78%;">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['product_title']); ?>
        </div>
      </div>
    </div>
  </div>

  <div class="uk-width-4-10">

    <div class="uk-grid uk-grid-collapse">


      <div class="uk-width-3-10">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['cost']); ?>
        </div>
      </div>

      <div class="uk-width-3-10">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['discount']); ?>
        </div>
      </div>

      <div class="uk-width-4-10">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['total']); ?>
        </div>
      </div>
    </div>

  </div>
</div>
