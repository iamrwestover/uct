<?php
$view_mode = !empty($form['#av_view_mode']);
?>
<div class="av-nestable-form uk-nestable av-nestable-product-list-form<?php print ($view_mode ? ' av-nestable-product-list-view' : ''); ?>">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase">
      <div class="uk-width-5-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-1-2 uk-text-center" style="width: 7%;"><div class="av-nestable-cell">#</div></div>
          <div class="uk-width-1-2" style="width: 93%;"><div class="av-nestable-cell">Product</div></div>
        </div>
      </div>

      <div class="uk-width-5-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-1-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Qty
            </div>
          </div>
          <div class="uk-width-3-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'left' : 'center'); ?>">
              UOM
            </div>
          </div>
          <div class="uk-width-2-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              <?php print ($form['#transaction'] == 'sales' ? t('Unit Price') : t('Unit Cost')); ?>
            </div>
          </div>
          <div class="uk-width-<?php print ($view_mode ? '2' : '1'); ?>-10 uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
            <div class="av-nestable-cell">
              DISC.%
            </div>
          </div>
          <div class="uk-width-2-10">
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

  <?php
    // Group item rows by reference type.
    $item_rows_by_ref_txn_type = array();
    $ttd = avtxns_txn_types();
    foreach (element_children($form) as $item_key) {
      $ref_txn_type = empty($form[$item_key]['#prod_ref_txn_type']) ? '-' : $form[$item_key]['#prod_ref_txn_type'];
      $item_rows_by_ref_txn_type[$ref_txn_type][$item_key] = $form[$item_key];
    }

    ksort($item_rows_by_ref_txn_type);
    $item_index = 0;
    foreach ($item_rows_by_ref_txn_type as $ref_txn_type => $item_rows) {
      print (($view_mode && $ref_txn_type == AVTXNS_TXN_TYPE_SALES_RETURN) ? '<div class="uk-badge uk-badge-warning uk-margin-small-top">' . strtoupper($ttd[$ref_txn_type]['name']) . '</div>' : '');
      foreach ($item_rows as $item_key => $item_row) {
        $item_rows[$item_key]['#prod_index'] = $item_index++;
      }
      print drupal_render($item_rows);
    }
  ?>
  <?php //print drupal_render_children($form); ?>
  <div id="item-list-new-product-wrapper"></div>
</div>
