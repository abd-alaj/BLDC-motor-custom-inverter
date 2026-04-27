function blkStruct = slblocks

blkStruct.Name    = 'filters library';
blkStruct.OpenFcn = 'filter_lib';
blkStruct.MaskInitialization = '';
blkStruct.MaskDisplay = '';

Browser.Library = 'filter_lib';
Browser.Name    = 'filters library';
Browser.IsFlat  = 0;

blkStruct.Browser = Browser;