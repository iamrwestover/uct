<?php
$view_mode = !empty($form['#av_view_mode']);
?>
<div class="av-nestable-form uk-nestable av-nestable-product-list-form<?php print ($view_mode ? ' av-nestable-product-list-view' : ''); ?>">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase">
      <div class="uk-width-5-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-1-1">
            <div class="av-nestable-cell">

              <div class="uk-grid uk-grid-collapse">
                <div class="uk-width-1-10">

                  <div class="uk-grid uk-grid-collapse uk-text-center">
                    <?php if (empty($view_mode)): ?>
                      <div class="uk-width-1-2">&nbsp;</div>
                    <?php endif; ?>
                    <div class="uk-width-<?php print $view_mode ? '2' : '1'; ?>-2">#</div>
                  </div>

                </div>

                <div class="uk-width-9-10 uk-text-left" style="padding-left: 30px;">Product</div>
              </div>

            </div>
          </div>
        </div>
      </div>

      <div class="uk-width-5-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-1-2">
            <div class="uk-grid uk-grid-collapse">
              <div class="uk-width-3-10">
                <div class="av-nestable-cell uk-text-center">
                  <span class="snot-printable">Stock</span>
                </div>
              </div>
              <div class="uk-width-3-10">
                <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
                  Qty
                </div>
              </div>
              <div class="uk-width-4-10">
                <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'left' : 'center'); ?>">
                  UOM
                </div>
              </div>
            </div>
          </div>

          <div class="uk-width-1-2">
            <div class="uk-grid uk-grid-collapse">
              <div class="uk-width-3-10">
                <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
                  <?php print ($form['#transaction'] == 'sales' ? t('Unit Price') : t('Unit Cost')); ?>
                </div>
              </div>
              <div class="uk-width-<?php print ($view_mode ? '3' : '2'); ?>-10 uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
                <div class="av-nestable-cell">
                  DISC.%
                </div>
              </div>
              <div class="uk-width-4-10">
                <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
                  Total
                </div>
              </div>
              <?php if (empty($view_mode)): ?>
                <div class="uk-width-1-10">
                  <div class="av-nestable-cell">
                    <!--Actions-->
                  </div>
                </div>
              <?php endif; ?>
            </div>
          </div>
        </div>
      </div>

    </div>
  </div>

  <?php
  // Group item rows by category title.
  $item_rows_by_category = array();
  foreach (element_children($form) as $item_key) {
    $category_title = empty($form[$item_key]['#prod_category']) ? '-' : $form[$item_key]['#prod_category'];
    $item_rows_by_category[$category_title][$item_key] = $form[$item_key];
  }

  $item_index = 0;
  foreach ($item_rows_by_category as $category_title => $item_rows) {
    print ($view_mode ? '<div class="uk-badge uk-badge-warning uk-margin-small-top">' . $category_title . '</div>' : '');
    foreach ($item_rows as $item_key => $item_row) {
      $item_rows[$item_key]['#prod_index'] = $item_index++;
    }
    print drupal_render($item_rows);
  }
  ?>
  <?php //print drupal_render_children($form); ?>
  <div id="item-list-new-product-wrapper"></div>
</div>
