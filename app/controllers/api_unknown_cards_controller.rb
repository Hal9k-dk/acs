class ApiUnknownCardsController < BaseApiController
  before_filter only: :create do
    unless @json.has_key?('card_id')
      logger.info "Rejected by filter"
      render nothing: true, status: :bad_request
    end
  end

  def create
    user = User.find_by_card_id(params[:card_id])
    if user && user.active
        # A user already has this card, return HTTP 409
        logger.info "User not found"
        render nothing: true, status: :conflict
    else
      @unknown_card = UnknownCard.new
      @unknown_card.card_id = params[:card_id]
      @unknown_card.stamp = Time.now
      if @unknown_card.save
        render json: @unknown_card
      else
        logger.info "Could not save unknown_card entry"
        render nothing: true, status: :bad_request
      end
    end
  end
end
