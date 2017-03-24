<?php
$view_mode = !empty($form['#av_view_mode']);
?>
<div class="av-nestable-form uk-nestable av-nestable-product-list-form<?php print ($view_mode ? ' av-nestable-product-list-view' : ''); ?>">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase">
      <div class="uk-width-6-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-1-2 uk-text-center" style="width: 7%;"><div class="av-nestable-cell">#</div></div>
          <div class="uk-width-1-2" style="width: 93%;"><div class="av-nestable-cell">Product</div></div>
        </div>
      </div>

      <div class="uk-width-4-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-1-4">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              OnHand Qty
            </div>
          </div>
          <div class="uk-width-1-4">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              New Qty
            </div>
          </div>
          <div class="uk-width-<?php print ($view_mode ? '2' : '1'); ?>-4">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Qty Diff.
            </div>
          </div>
          <?php if (empty($view_mode)): ?>
            <div class="uk-width-1-4">
              <div class="av-nestable-cell">
                <!--Actions-->
              </div>
            </div>
          <?php endif; ?>
        </div>
      </div>

    </div>
  </div>

  <?php
  // Group item rows by category title.
  $item_rows_by_category = array();
  foreach (element_children($form) as $item_key) {
    $category_title = empty($form[$item_key]['#prod_category']) ? t('No Category') : $form[$item_key]['#prod_category'];
    $category_title = strtoupper($category_title);
    $item_rows_by_category[$category_title][$item_key] = $form[$item_key];
  }

  $item_index = 0;
  foreach ($item_rows_by_category as $category_title => $item_rows) {
    print ($view_mode ? '<div class="uk-badge uk-badge-warning">' . $category_title . '</div>' : '');
    foreach ($item_rows as $item_key => $item_row) {
      $item_rows[$item_key]['#prod_index'] = $item_index++;
    }
    print drupal_render($item_rows);
  }
  ?>
  <?php //print drupal_render_children($form); ?>
  <div id="item-list-new-product-wrapper"></div>
</div>
