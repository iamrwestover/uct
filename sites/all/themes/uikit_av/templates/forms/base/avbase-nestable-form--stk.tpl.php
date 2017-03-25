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
          <div class="uk-width-1-5">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              OnHand
            </div>
          </div>
          <div class="uk-width-1-5">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              New Qty
            </div>
          </div>
          <div class="uk-width-1-5">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Qty Diff.
            </div>
          </div>
          <div class="uk-width-<?php print ($view_mode ? '2' : '1'); ?>-5">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'left' : 'center'); ?>">
              Base UOM
            </div>
          </div>
          <?php if (empty($view_mode)): ?>
            <div class="uk-width-1-5">
              <div class="av-nestable-cell">
                <!--Actions-->
              </div>
            </div>
          <?php endif; ?>
        </div>
      </div>

    </div>
  </div>

  <?php print drupal_render_children($form); ?>
  <div id="item-list-new-product-wrapper"></div>
</div>
