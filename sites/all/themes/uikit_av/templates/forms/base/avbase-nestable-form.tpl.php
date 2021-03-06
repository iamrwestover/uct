<?php
$view_mode = !empty($form['#av_view_mode']);
?>
<div class="av-nestable-form uk-nestable av-nestable-product-list-form<?php print ($view_mode ? ' av-nestable-product-list-view' : ''); ?>">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase">
      <div class="uk-width-5-10">
        <div class="uk-grid uk-grid-collapse">
          <?php if (!$view_mode): ?>
            <div class="uk-width-1-2 uk-text-center" style="width: 7%;"><div class="av-nestable-cell">#</div></div>
          <?php endif; ?>
          <div class="uk-width-1-2" style="width: <?php print ($view_mode ? '100' : '93'); ?>%;"><div class="av-nestable-cell">Product</div></div>
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
    avtxns_group_and_sort_item_list($form, $view_mode);
  ?>
  <?php //print drupal_render_children($form); ?>
  <div id="item-list-new-product-wrapper"></div>
</div>
