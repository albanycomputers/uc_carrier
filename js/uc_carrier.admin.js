/**
 * @file
 * Price grid: switching service, with a warning about unsaved changes.
 */
(function ($) {
  'use strict';

  Backdrop.behaviors.ucCarrierServiceSelect = {
    attach: function (context) {
      $('.uc-carrier-service-select', context).once('uc-carrier-service').each(function () {
        var $select = $(this);
        var $form = $select.closest('form');
        var current = $select.val();

        $form.find('.uc-carrier-prices-grid input').on('change', function () {
          $form.data('ucCarrierDirty', true);
        });

        $select.on('change', function () {
          if ($form.data('ucCarrierDirty') && !window.confirm(Backdrop.t('You have unsaved price changes. Switch service and lose them?'))) {
            $select.val(current);
            return;
          }
          var url = $select.data('url');
          window.location = url + (url.indexOf('?') === -1 ? '?' : '&') + 'service=' + encodeURIComponent($select.val());
        });
      });
    }
  };

})(jQuery);
