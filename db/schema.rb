# This file is auto-generated from the current state of the database. Instead
# of editing this file, please use the migrations feature of Active Record to
# incrementally modify your database, and then regenerate this schema definition.
#
# Note that this schema.rb definition is the authoritative source for your
# database schema. If you need to create the application database on another
# system, you should be using db:schema:load, not running all the migrations
# from scratch. The latter is a flawed and unsustainable approach (the more migrations
# you'll amass, the slower it'll run and the greater likelihood for issues).
#
# It's strongly recommended that you check this file into your version control system.

ActiveRecord::Schema.define(version: 0) do

  create_table "logs", id: :integer, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.datetime "stamp",     default: -> { "CURRENT_TIMESTAMP" }, null: false
    t.string   "message"
    t.integer  "logger_id",                                      null: false
    t.integer  "user_id"
  end

  create_table "machines", id: :integer, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.string   "name"
    t.datetime "created_at", default: -> { "CURRENT_TIMESTAMP" }, null: false
    t.datetime "updated_at",                                      null: false
    t.string   "api_token"
  end

  create_table "machines_users", id: false, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.integer "machine_id", null: false
    t.integer "user_id",    null: false
  end

  create_table "old_machines_users", id: false, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.integer "machine_id", null: false
    t.integer "user_id",    null: false
  end

  create_table "old_permissions_users", id: false, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.integer "permission_id", null: false
    t.integer "user_id",       null: false
  end

  create_table "permissions", id: :integer, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.string   "name"
    t.datetime "created_at", default: -> { "CURRENT_TIMESTAMP" }, null: false
    t.datetime "updated_at",                                      null: false
  end

  create_table "permissions_users", id: false, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.integer "permission_id", null: false
    t.integer "user_id",       null: false
  end

  create_table "unknown_cards", id: :integer, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.datetime "stamp",      default: -> { "CURRENT_TIMESTAMP" }, null: false
    t.string   "card_id"
    t.datetime "created_at",                                      null: false
    t.datetime "updated_at",                                      null: false
  end

  create_table "users", id: :integer, force: :cascade, options: "ENGINE=MyISAM DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci" do |t|
    t.integer "fl_id"
    t.integer "member_id"
    t.string  "display_name"
    t.boolean "active"
    t.string  "card_id"
    t.string  "name"
    t.string  "login"
    t.string  "password_digest"
  end

end
