<?php
foreach (element_children($form) as $key) {
  $form[$key]['#title_display'] = 'invisible';
}
$form['qty']['#attributes']['class'][] = 'uk-text-right';
$form['price']['#attributes']['class'][] = 'uk-text-right';
$form['amount']['#attributes']['class'][] = 'uk-text-right';
?>
<div class="uk-nestable-item">
  <div class="uk-grid uk-grid-collapse">
    <div class="uk-width-8-10">

      <div class="uk-grid uk-grid-collapse">
        <div class="uk-width-4-5">
          <div class="av-nestable-cell">

              <div class="uk-grid uk-grid-collapse">
                <div class="uk-width-1-10">

                  <div class="uk-grid uk-grid-collapse">
                    <div class="uk-width-1-2"><i class="uk-nestable-handle uk-icon uk-icon-bars uk-margin-small-right"></i></div>
                    <div class="uk-width-1-2"><span class="av-nestable-row-num"><?php print $form['#prod_index'] + 1; ?></span></div>
                  </div>

                </div>

                <div class="uk-width-9-10"><?php print drupal_render($form['product_id']); ?></div>
              </div>

          </div>
        </div>
        <div class="uk-width-1-5">
          <div class="av-nestable-cell">
            <?php print drupal_render($form['uom_id']); ?>
          </div>
        </div>
      </div>
    </div>

    <div class="uk-width-2-10">

      <div class="uk-grid uk-grid-collapse">
        <div class="uk-width-1-5">
          <div class="av-nestable-cell">
            <?php print drupal_render($form['qty']); ?>
          </div>
        </div>

        <div class="uk-width-2-5">
          <div class="av-nestable-cell">
            <?php print drupal_render($form['price']); ?>
          </div>
        </div>

        <div class="uk-width-2-5">
          <div class="av-nestable-cell">
            <?php print drupal_render($form['amount']); ?>
          </div>
        </div>
      </div>

    </div>
  </div>
</div>
