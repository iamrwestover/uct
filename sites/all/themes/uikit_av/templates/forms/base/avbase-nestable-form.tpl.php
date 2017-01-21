<div class="av-nestable-form uk-nestable av-nestable-product-list-form">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase">
      <div class="uk-width-7-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-4-5">
            <div class="av-nestable-cell">

              <div class="uk-grid uk-grid-collapse">
                <div class="uk-width-1-10">

                  <div class="uk-grid uk-grid-collapse uk-text-center">
                    <div class="uk-width-1-2">&nbsp;</div>
                    <div class="uk-width-1-2">#</div>
                  </div>

                </div>

                <div class="uk-width-9-10 uk-text-left" style="padding-left: 30px;">Product</div>
              </div>

            </div>
          </div>
          <div class="uk-width-1-5">
            <div class="av-nestable-cell uk-text-left" style="padding-left: 1em;">
              UOM
            </div>
          </div>
        </div>
      </div>

      <div class="uk-width-3-10 uk-text-right">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-1-6">
            <div class="av-nestable-cell" style="padding-right: 1em;">
              Qty
            </div>
          </div>
          <div class="uk-width-2-6">
            <div class="av-nestable-cell" style="padding-right: 1em;">
              <?php print ($form['#transaction'] == 'sales' ? t('Unit Price') : t('Unit Cost')); ?>
            </div>
          </div>
          <div class="uk-width-2-6">
            <div class="av-nestable-cell" style="padding-right: 1em;">
              Total
            </div>
          </div>
        </div>
        <div class="uk-width-1-6">
          <div class="av-nestable-cell">
            <!--Actions-->
          </div>
        </div>
      </div>

    </div>
  </div>

  <?php print drupal_render_children($form); ?>
  <div id="item-list-new-product-wrapper"></div>
</div>
