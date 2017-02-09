<?php
$view_mode = !empty($form['#av_view_mode']);
?>
<div class="av-nestable-form uk-nestable av-nestable-product-list-form<?php print ($view_mode ? ' av-nestable-product-list-view' : ''); ?>">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase">
      <div class="uk-width-3-10">
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

                <div class="uk-width-6-10" style="padding-left: 30px;">Invoice Date</div>
              </div>

            </div>
          </div>
        </div>
      </div>

      <div class="uk-width-7-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-2-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'left' : 'center'); ?>">
              Invoice ID
            </div>
          </div>

          <div class="uk-width-2-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Invoice Total
            </div>
          </div>
          <div class="uk-width-2-10">
            <div class="av-nestable-cell uk-text-<?php print ($view_mode ? 'right' : 'center'); ?>">
              Prev. Payment
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
        </div>
      </div>

    </div>
  </div>

  <?php print drupal_render_children($form); ?>
  <div id="item-list-new-product-wrapper"></div>
</div>
