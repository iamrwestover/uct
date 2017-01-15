<?php
foreach (element_children($form) as $key) {
  $form[$key]['#title_display'] = 'invisible';
}

//$form['product_title']['#attributes']['placeholder'] = 'product code or name';

$form['qty']['#attributes']['class'][] = 'uk-text-right';
$form['price']['#attributes']['class'][] = 'uk-text-right';
$form['amount']['#attributes']['class'][] = 'uk-text-right';

// Style delete btn.
$form['prod_delete_btn']['#button_label'] = '<i class="uk-icon-remove"></i>';
$form['prod_delete_btn']['#attributes']['class'][] = 'av-ajax-trigger';
$form['prod_delete_btn']['#attributes']['class'][] = 'av-ajax-trigger-icon-only';
$form['prod_delete_btn']['#attributes']['class'][] = 'uk-button-danger';
$form['prod_delete_btn']['#attributes']['class'][] = 'uk-button-mini';
//$form['prod_delete_btn']['#attributes']['class'][] = 'uk-text-muted';

?>

<div class="uk-grid uk-grid-collapse">
  <div class="uk-width-7-10">

    <div class="uk-grid uk-grid-collapse">
      <div class="uk-width-4-5">
        <div class="av-nestable-cell">

            <div class="uk-grid uk-grid-collapse">
              <div class="uk-width-1-10">

                <div class="uk-grid uk-grid-collapse uk-text-center">
                  <div class="uk-width-1-2"><i class="uk-nestable-handle uk-icon uk-icon-bars uk-margin-small-right"></i></div>
                  <div class="uk-width-1-2"><span class="av-nestable-row-num"><?php print $form['#prod_index'] + 1; ?></span></div>
                </div>

              </div>

              <div class="uk-width-9-10"><?php print drupal_render($form['product_title']); ?></div>
            </div>

        </div>
      </div>
      <div class="uk-width-1-5">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['uom_title']); ?>
          <?php print drupal_render($form['qty_per_uom']); ?>
        </div>
      </div>
    </div>
  </div>

  <div class="uk-width-3-10">

    <div class="uk-grid uk-grid-collapse">
      <div class="uk-width-1-6">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['qty']); ?>
        </div>
      </div>

      <div class="uk-width-2-6">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['price']); ?>
        </div>
      </div>

      <div class="uk-width-2-6">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['amount']); ?>
        </div>
      </div>

      <div class="uk-width-1-6 uk-text-center">
        <div class="av-nestable-cell">
          <?php print drupal_render($form['prod_delete_btn']); ?>
        </div>
      </div>
    </div>

  </div>
</div>
