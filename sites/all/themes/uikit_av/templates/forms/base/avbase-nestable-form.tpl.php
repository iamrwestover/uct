<div class="av-nestable-form uk-nestable av-nestable-product-list-form" data-uk-nestable="{handleClass:'uk-nestable-handle', maxDepth: 1}">
  <div class="av-nestable-form-header">
    <div class="uk-grid uk-grid-collapse uk-text-bold uk-text-uppercase uk-text-center">
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

                <div class="uk-width-9-10">Product</div>
              </div>

            </div>
          </div>
          <div class="uk-width-1-5">
            <div class="av-nestable-cell">
              UOM
            </div>
          </div>
        </div>
      </div>

      <div class="uk-width-3-10">
        <div class="uk-grid uk-grid-collapse">
          <div class="uk-width-2-6">
            <div class="av-nestable-cell">
              Price
            </div>
          </div>
          <div class="uk-width-1-6">
            <div class="av-nestable-cell">
              Qty.
            </div>
          </div>
          <div class="uk-width-2-6">
            <div class="av-nestable-cell">
              Amount
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
  <div id="po-new-product-wrapper"></div>
</div>
