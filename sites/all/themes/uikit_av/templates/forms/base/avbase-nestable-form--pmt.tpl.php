<?php
$view_mode = !empty($form['#av_view_mode']);
?>
<div class="av-nestable-form uk-nestable av-nestable-product-list-form<?php print ($view_mode ? ' av-nestable-product-list-view' : ''); ?>">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase">
      <div class="uk-width-4-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-2-6"><div class="av-nestable-cell">Date</div></div>
          <div class="uk-width-2-6">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'left' : 'center'); ?>">
              Txn#
            </div>
          </div>
          <div class="uk-width-2-6">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'left' : 'center'); ?>">
              Ref. Txn#
            </div>
          </div>
        </div>
      </div>

      <div class="uk-width-6-10">
        <div class="uk-grid uk-grid-collapse">

          <div class="uk-width-3-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Total
            </div>
          </div>
          <div class="uk-width-<?php print ($view_mode ? '3' : '2'); ?>-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Total Paid
            </div>
          </div>
          <div class="uk-width-2-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Balance
            </div>
          </div>
          <div class="uk-width-2-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Payment
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

  <?php print drupal_render_children($form); ?>
  <div id="item-list-new-product-wrapper"></div>
</div>
