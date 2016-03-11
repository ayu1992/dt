function getResizedBoundingBoxes(videoIndex, ucfCategory, IVCategory) 
% ucfCategory: Diving-Side/
% IVCategory: Diving/
% videoIndex: 1
    if videoIndex < 10
        prefix = '/00';
    else 
        prefix = '/0';
    end
    
    ucfFilesPath = strcat(ucfCategory, strcat( strcat(prefix, int2str(videoIndex)),'/'));
    originalImgs = dir(strcat(ucfFilesPath,'*.jpg'));
    image = imread(strcat(ucfFilesPath, originalImgs(1).name));
    [height,width,z] = size(image);
    
    annotationsPath = strcat('Annotations/',strcat(ucfCategory, strcat(prefix, strcat(int2str(videoIndex),'/gt/'))));
    boxes = dir(strcat(annotationsPath, '*.txt'));
    
    ivPath = strcat('../../dense_trajectory/InputVideos/',strcat(IVCategory, '/'));
    resizedVideoInfo = aviinfo(strcat(ivPath, strcat(int2str(videoIndex),'.avi')));
    
    widthChange = resizedVideoInfo.Width / width
    heightChange = resizedVideoInfo.Height / height
     
    fout = fopen(strcat(ivPath, strcat(int2str(videoIndex), '.txt')), 'w');
    formatSpec = '%d %d %d %d\n';
    
    for box = boxes'
        fid = fopen(strcat(annotationsPath, box.name));
        val = textscan(fid, '%d %d %d %d %*s');
        %fprintf('%d %d %d %d', val(1] * widthChange, val[2] * heightChange, val[3] * widthChange, val[4] * heightChange);
        nx = val{1} * widthChange;
        ny = val{2} * heightChange;
        nwidth = val{3} * widthChange;
        nheight = val{4} * heightChange;
        fprintf(fout, formatSpec, [nx, ny, nwidth, nheight]);
        fclose(fid);
    end
end