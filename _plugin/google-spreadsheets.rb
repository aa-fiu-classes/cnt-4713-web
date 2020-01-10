require 'google_drive'
require 'fileutils'
require 'time'
require 'date'
require 'yaml'

module Jekyll
  class ProgramData < Generator
    def generate(site)
      if site.config['spreadsheets'] == nil
        return Jekyll.logger.warn "Google spreadsheet data not generated, because 'spreadsheets' is not defined in _config.yml"
      end

      cacheDir = "#{site.source}/.jekyll-cache"
      if not File.directory? cacheDir
        FileUtils::mkdir_p cacheDir
      end

      if site.config['spreadsheets']['google_client_id'] == nil
        return Jekyll.logger.warn "Google spreadsheet data not generated, because 'spreadsheets.google_client_id' is not defined in _config.yml"
      end

      if site.config['spreadsheets']['spreadsheet'] == nil
        return Jekyll.logger.warn "Google spreadsheet data not generated, because 'spreadsheets.spreadsheet' is not defined in _config.yml"
      end

      begin
        session = GoogleDrive::Session.from_config(site.config['spreadsheets']['google_client_id'])
        Jekyll.logger.info 'Generating data from Google spreadsheets...'
        site.data['spreadsheets'] = {}

        for sheetKey in site.config['spreadsheets']['spreadsheet']
          begin
            Jekyll.logger.warn '   from spreadsheet ', sheetKey
            spreadsheet = session.spreadsheet_by_key(sheetKey)

            for ws in spreadsheet.worksheets
              begin
                list = []
                ws.list.each do |item|
                  list << item.to_hash
                end

                site.data['spreadsheets'][ws.title] = list
                # File.write file, list.to_yaml
              rescue
                Jekyll.logger.warn "Error processing worksheet: ", $!
              end
            end
          rescue
            Jekyll.logger.warn "Error processing spreadsheet: ", $!
          end
        end
      rescue
        return Jekyll.logger.error "Failed to process Google spreadsheets", $!
      end
    end
  end
end
