class ApiLogsController < BaseApiController
  before_filter only: :create do
    unless @json.has_key?('log') && @json['log'].respond_to?(:[]) && @json['log']['message']
      logger.info "Rejected by filter"
      render nothing: true, status: :bad_request
    end
  end

  def create
    if !@machine
      logger.info "Machine not found"
      render nothing: true, status: :forbidden
    else
      a = @json['log']
      @log = Log.new
      @log.assign_attributes(a)
      @log.logger_id = @machine.id
      if @json['log']['user_id']
        @log.user_id = @json['log']['user_id']
      end
      if @log.save
        render json: @log
      else
        logger.info "Could not save log entry"
        render nothing: true, status: :bad_request
      end
    end
  end

  def show
    if !@machine
      logger.info "Machine not found"
      render nothing: true, status: :forbidden
    else
      @log = Log.where("logger_id = #{@machine.id}").order("id desc").
             joins("LEFT JOIN users ON users.id = logs.user_id").
             select("logs.*, users.name AS user_name")
      if params['user_id']
        @log = @log.where("user_id = #{params['user_id']}")
      end
      if params['last']
        @log = @log.limit(params['last'])
      end
      render json: @log
    end
  end
end
