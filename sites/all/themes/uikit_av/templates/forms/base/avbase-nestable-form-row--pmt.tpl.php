<?php
$view_mode = !empty($form['#av_view_mode']);
//$show_discount_field =
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
  <div class="uk-width-4-10">

    <div class="uk-grid uk-grid-collapse">
      <div class="uk-width-2-6">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['transaction_date_formatted']); ?>
        </div>
      </div>
      <div class="uk-width-2-6">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['ref_txn_id_text']); ?>
          <?php print drupal_render($form['ref_txn_id']); ?>
        </div>
      </div>
      <div class="uk-width-2-6">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['items_ref_txn_id_text']); ?>
        </div>
      </div>
    </div>
  </div>

  <div class="uk-width-6-10">

    <div class="uk-grid uk-grid-collapse">

      <div class="uk-width-3-10">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['orig_total']); ?>
        </div>
      </div>
      <div class="uk-width-<?php print ($view_mode ? '3' : '2'); ?>-10">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['previous_payment']); ?>
        </div>
      </div>
      <div class="uk-width-2-10">
        <div class="av-nestable-cell uk-text-right<?php print($view_mode ? ' uk-text-warning' : ''); ?>">
          <?php print drupal_render($form['balance']); ?>
        </div>
      </div>
      <div class="uk-width-2-10">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['total']); ?>
        </div>
      </div>
      <?php if (empty($view_mode)): ?>
        <div class="uk-width-1-10 uk-text-center">
          <div class="av-nestable-cell">
            <?php print drupal_render($form['prod_delete_btn']); ?>
          </div>
        </div>
      <?php endif; ?>
    </div>

  </div>
</div>
