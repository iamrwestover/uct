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

<div class="uk-grid uk-grid-collapse" style="position: relative;">
  <?php print drupal_render($form['badge']); ?>
  <div class="uk-width-4-10">

    <div class="uk-grid uk-grid-collapse">
      <div class="uk-width-1-2 uk-text-center" style="width: 7%;">
        <div class="av-nestable-cell">
          <span class="av-nestable-row-num" style="<?php print $view_mode ? 'top: 0;' : ''; ?>"><?php print $form['#prod_index'] + 1; ?></span>
        </div>
      </div>
      <div class="uk-width-1-2" style="width: 93%;">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['account_name']); ?>
        </div>
      </div>
    </div>
  </div>

  <div class="uk-width-6-10">

    <div class="uk-grid uk-grid-collapse">
      <div class="uk-width-1-10">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['debit']); ?>
        </div>
      </div>

      <div class="uk-width-1-10">
        <div class="av-nestable-cell uk-text-right">
          <?php print drupal_render($form['credit']); ?>
        </div>
      </div>

      <div class="uk-width-<?php print ($view_mode ? '4' : '3'); ?>-10">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['description']); ?>
        </div>
      </div>

      <div class="uk-width-4-10">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['client_name']); ?>
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
