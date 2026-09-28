global record TargetSpec
   instrument: string
   group: string | nil
   channel: integer | nil
end

global record Target
   get_instrument_name: function(self: Target): string
   get_channel_group: function(self: Target): string
   get_channel: function(self: Target): number
   clone: function(self: Target): Target
   to_string: function(self: Target): string
end

global record instrument_target
   new: function(TargetSpec): Target
end
