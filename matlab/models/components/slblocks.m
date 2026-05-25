function blkStruct = slblocks

blkStruct.Name    = 'inverter library';
blkStruct.OpenFcn = 'inverter_library';
blkStruct.MaskInitialization = '';
blkStruct.MaskDisplay = '';

Browser.Library = 'inverter_library';
Browser.Name    = 'inverter library';
Browser.IsFlat  = 0;

blkStruct.Browser = Browser;