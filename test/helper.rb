require "rubygems"
require "bundler/setup"

require 'erb'
require 'minitest/unit'
require 'minitest/autorun'

require 'ruby-debug'

require 'libssh'
require_relative 'support/string'
require_relative 'support/ssh_daemon'
