FROM ruby:3.0 

RUN apt-get update \
  && apt-get install -y nodejs \
  && apt-get install sudo \
  && apt-get clean \
  && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

RUN mkdir -p /app
WORKDIR /app

COPY ./ /app
RUN cd /app && gem install rails && gem install bundler && bundle update
