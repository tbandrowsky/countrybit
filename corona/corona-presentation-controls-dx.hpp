/*
CORONA
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024 + All Rights Reserved


MIT License

About this File
This is for controls using direct2d to draw their own contexts.

Notes

For Future Consideration
*/


#ifndef CORONA_PRESENTATION_CONTROLS_DX_H
#define CORONA_PRESENTATION_CONTROLS_DX_H


namespace corona
{

    class draw_control : public control_base
    {

    private:

    public:
        std::shared_ptr<viewStyleRequest>	view_style;

        std::weak_ptr<applicationBase> host;
        std::weak_ptr<direct2dChildWindow> window;
        std::function<void(std::shared_ptr<direct2dContext>& _context, draw_control*)> on_draw;
        std::function<void(std::shared_ptr<direct2dContext>& _context, draw_control*)> on_create;

        lockable camera_access_lock;

        draw_control()
        {
            parent = nullptr;
            id = id_counter::next();
        }

        draw_control(const draw_control& _src) : control_base(_src)
        {
            window = _src.window;
            host = _src.host;
            view_style = _src.view_style;
            on_draw = _src.on_draw;
            on_create = _src.on_create;
        }

        draw_control(container_control_base* _parent, int _id)
        {
            parent = _parent;
            id = _id;
        }

        virtual ~draw_control()
        {
        }

        virtual void create(std::weak_ptr<applicationBase> _host)
        {
            host = _host;

            if (auto phost = _host.lock()) {
                if (bounds.x < 0 or bounds.y < 0 or bounds.w < 0 or bounds.h < 0) {
                    std::stringstream ss;
                    ss << typeid(*this).name() << " bounds is jacked on create";
                    std::string mm = ss.str();
                    system_monitoring_interface::global_mon->log_warning(mm);
                }

                bool use_native_window = false; // a placeholder, and this is a critical spot 
                // for it, but you'll need to do a few other things as well.
                // and you can always implement for your own window
                // easily enough.
                if (use_native_window) {
                    window = phost->createDirect2Window(id, bounds);
                    auto pwindow = window.lock();
                    if (pwindow) {
                        auto context = pwindow->getContext();
                        on_create(context, this);
                    }
                }
                else if (on_create)
                {
                    if (auto pwindow = phost->getWindow().lock())
                    {
                        if (auto context = pwindow->getContext())
                        {
                            on_create(context, this);
                        }
                    }
                }


                //				std::cout << this << ":" << typeid(*this).name() << " created." << std::endl;
            }
            /*			else
                        {
                            std::stringstream ss;
                            ss << typeid(*this).name() << " not created because the host is not available";
                            system_monitoring_interface::global_mon->log_warning(ss.str());
                        }
                        */
            for (auto child : children) {
                child->create(_host);
            }
        }

        void destroy()
        {
            for (auto child : children) {
                if (child) {
                    child->destroy();
                }
            }
        }

        void on_resize()
        {
            auto ti = typeid(*this).name();

            if (auto pwindow = window.lock())
            {
                pwindow->moveWindow(inner_bounds.x, inner_bounds.y, inner_bounds.w, inner_bounds.h);
            }
        }

        virtual bool is_camera() { return false; }

        void render(std::shared_ptr<direct2dContext>& _context)
        {
            D2D1_COLOR_F color = {};

            std::string border_name;
            std::string background_name;

            if (view_style) {
                auto vs = *view_style;
                point pt;
                pt.x = bounds.w;
                pt.y = bounds.h;
                vs.apply_scale(pt);
                _context->setViewStyle(vs);
            }

            if (view_style and view_style->box_border_brush.get_name())
            {
                border_name = view_style->box_border_brush.get_name();
            }

            if (view_style and view_style->box_fill_brush.get_name())
            {
                background_name = view_style->box_fill_brush.get_name();
            }

            if (background_name.size()) {
                rectangle r = inner_bounds;
                //							std::cout << std::format("{}:{} [{},{} x {},{}]", typeid(*this).name(), background_name, r.x, r.y, r.w, r.h) << std::endl;
                _context->drawRectangle(&r, "", 0, background_name);
            }

            /*
            * this is commented out because it is helpful for debugging.
            * a parent is always the background of its children,
            * so, what this does is slightly alter a nasty pink
            * color so you can see what the shape of all the things are
            * that do not have backgrounds.  This is helpful for layout work and debugging.
            */

            if (on_draw)
            {
                on_draw(_context, this);
            }

            if (border_name.size()) {
                rectangle r = inner_bounds;
                //							std::cout << std::format("{}:{} [{},{} x {},{}]", typeid(*this).name(), background_name, r.x, r.y, r.w, r.h) << std::endl;
                _context->drawRectangle(&r, border_name, view_style->box_border_thickness, "");
            }

            for (auto& child : children)
            {
                child->render(_context);
            }
        }

        void set_style(std::shared_ptr<viewStyleRequest> _style)
        {
            view_style = _style;
        }

        virtual void get_json(json& _dest)
        {
            json_parser jp;
            control_base::get_json(_dest);

            if (view_style) {
                json jview_style = jp.create_object();
                corona::get_json(jview_style, *view_style);
            }

            json issues = _dest["issues"];
            if (issues.array()) {
                if (window.expired()) {
                    issues.push_back("d2d window is not created");
                }
            }
        }

        virtual void put_json(json& _src)
        {
            control_base::put_json(_src);

            json jview_style = _src["view_style"];
            if (jview_style.object()) {

                if (view_style)
                    view_style = std::make_shared<viewStyleRequest>(*view_style);
                else
                    view_style = std::make_shared<viewStyleRequest>();

                corona::put_json(view_style, jview_style);
            }
        }

    };

    class camera_control : public draw_control
    {

    private:
        HRESULT configure_decoder(IMFSourceReader* _pSourceReader, DWORD dwStreamIndex)
        {
            IMFMediaType* pNativeType = nullptr;
            IMFMediaType* pType = nullptr;
            GUID majorType, subtype;
            HRESULT hr;

            // Find the native format of the stream.
            hr = _pSourceReader->GetNativeMediaType(dwStreamIndex, 0, &pNativeType);

            if (SUCCEEDED(hr) and pNativeType) {

                // Find the major type.
                hr = pNativeType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);

                // Define the output type.
                hr = MFCreateMediaType(&pType);

                if (SUCCEEDED(hr) and pType) {

                    hr = pType->SetGUID(MF_MT_MAJOR_TYPE, majorType);

                    // Select a subtype.
                    if (majorType == MFMediaType_Video)
                    {
                        subtype = MFVideoFormat_ARGB32;
                    }
                    else if (majorType == MFMediaType_Audio)
                    {
                        subtype = MFAudioFormat_Float;
                    }
                    else
                    {
                        // Unrecognized type. Skip.
                        return S_FALSE;
                    }

                    hr = pType->SetGUID(MF_MT_SUBTYPE, subtype);

                    // Set the uncompressed format.
                    hr = _pSourceReader->SetCurrentMediaType(dwStreamIndex, nullptr, pType);
                    return hr;
                }
            }
            else
            {
                system_monitoring_interface::global_mon->log_warning("Couldn't get the native media type for the camera");
                return HRESULT_FROM_NT(E_FAIL);
            }
            return HRESULT_FROM_NT(E_FAIL);
        }

        IMFMediaSource* pSource;
        IMFSourceReader* pSourceReader;
        LONGLONG			stream_base_time;
        LONGLONG			last_barcode_time;
        delta_frame			delta_boi;
        int counter = 0;
        timer						camera_check_timer;
        std::string					camera_status;
        std::string					selected_camera_name;
        std::string					current_camera_name;
        std::vector<std::string>	available_cameras;

        void init()
        {
            on_draw = [this](std::shared_ptr<direct2dContext>& _context, draw_control* _dc) -> void {

                counter++;

                auto pwindow = window.lock();

                if (auto phost = host.lock()) {
                    auto draw_bounds = inner_bounds;

                    ID2D1DeviceContext* dc = _context->getDeviceContext();

                    double xmag = {}, ymag = {};
                    double mag = {};
                    D2D1_RECT_F dest_rect = {};
                    bool size_inited = false;

                    ID2D1Bitmap1* cbm = delta_boi.get_frame(dc);
                    if (cbm) {

                        auto surface_size = cbm->GetSize();

                        xmag = (draw_bounds.w / surface_size.width);
                        ymag = (draw_bounds.h / surface_size.height);

                        if (xmag > ymag)
                        {
                            mag = xmag;
                        }
                        else
                        {
                            mag = ymag;
                        }

                        dest_rect.left = draw_bounds.x;
                        dest_rect.top = draw_bounds.y;
                        dest_rect.right = surface_size.width * mag;
                        dest_rect.bottom = surface_size.height * mag;

                        dc->DrawBitmap(cbm, &dest_rect, 1.0);

                        cbm->Release();
                        size_inited = true;
                    }

                    sprinkle_buffer new_sprinkles;
                    ID2D1Bitmap1* dbm = delta_boi.get_activation(dc, new_sprinkles);
                    add_sprinkles(new_sprinkles);

                    if (size_inited and dbm)
                    {
                        dc->DrawBitmap(dbm, &dest_rect, 1.0);
                        dbm->Release();
                    }

                    auto boxes = get_movement_boxes();
                    if (boxes.size()) {
                        ID2D1SolidColorBrush* brush = nullptr;

                        D2D1_COLOR_F brushColor;
                        brushColor = toColor("#0000AA");
                        dc->CreateSolidColorBrush(brushColor, &brush);

                        solidBrushRequest sbr;
                        sbr.brushColor = brushColor;
                        sbr.name = "polycolor";
                        generalBrushRequest gbr(sbr);
                        _context->setBrush(&gbr);

                        if (brush)
                        {
                            int ct = 0;

                            for (auto& br : boxes)
                            {
                                pathImmediateDto ptd;

                                for (auto pt : br.left)
                                {
                                    pt.x = (br.area.x + pt.x) * mag + draw_bounds.x;
                                    pt.y = (br.area.y + pt.y) * mag + draw_bounds.y;
                                    ptd.path.addLineTo(pt.x, pt.y);
                                }

                                ptd.borderBrushName = "polycolor";
                                ptd.strokeWidth = 4;
                                ptd.rotation = 0;
                                ptd.closed = false;
                                ptd.position.x = 0;
                                ptd.position.y = 0;
                                _context->drawPath(&ptd);

                                ptd.path.points.clear();
                                for (auto pt : br.right)
                                {
                                    pt.x = (br.area.x + pt.x) * mag + draw_bounds.x;
                                    pt.y = (br.area.y + pt.y) * mag + draw_bounds.y;
                                    ptd.path.addLineTo(pt.x, pt.y);
                                }

                                ptd.borderBrushName = "polycolor";
                                _context->drawPath(&ptd);


                                /*									D2D1_RECT_F rect;
                                                                rect.left = br.area.x * mag + draw_bounds.x;
                                                                rect.top = br.area.y * mag + draw_bounds.y;
                                                                rect.right = br.area.right() * mag + draw_bounds.x;
                                                                rect.bottom = br.area.bottom() * mag + draw_bounds.y;
                                                                dc->DrawRectangle(rect, brush, 2.0);*/
                            }
                            brush->Release();
                        }
                    }
                }

                if (auto phost = host.lock()) {
                    auto draw_bounds = inner_bounds;

                    draw_bounds.x = inner_bounds.x;
                    draw_bounds.y = inner_bounds.y;

                    auto& context = _context;

                    auto st = presentation_style_factory::get_current()->get_style();

                    if (camera_status.size()) {

                        _context->setBrush(&st->CaptionStyle->shape_fill_brush);
                        auto temp = st->TitleStyle->text_style;
                        temp.name = "CameraStatus";
                        temp.vertical_align = visual_alignment::align_center;
                        temp.horizontal_align = visual_alignment::align_center;
                        _context->setTextStyle(&temp);
                        _context->drawText(camera_status, &draw_bounds, temp.name, st->CaptionStyle->shape_fill_brush.get_name());
                    }

                    if (current_camera_name.size()) {

                        _context->setBrush(&st->TitleStyle->shape_fill_brush);
                        auto temp = st->CodeStyle->text_style;
                        temp.name = "CameraName";
                        temp.vertical_align = visual_alignment::align_far;
                        temp.horizontal_align = visual_alignment::align_center;
                        _context->setTextStyle(&temp);
                        draw_bounds.h -= 10.0;
                        _context->drawText(current_camera_name, &draw_bounds, temp.name, st->TitleStyle->shape_fill_brush.get_name());
                    }

                }
            };

            on_create = [this](std::shared_ptr<direct2dContext>& _context, draw_control* _ctrl) ->void
                {
                    comm_bus_app_interface::global_bus->run_ui([this]() ->void {
                        start();
                        });
                };
        }

    public:

        camera_control()
        {
            pSource = nullptr;
            pSourceReader = nullptr;
            stream_base_time = 0;
            last_barcode_time = 0;
            camera_status = "Starting";
            init();
        }

        camera_control(const camera_control& _src) : draw_control(_src)
        {
            pSource = _src.pSource;
            if (pSource) {
                pSource->AddRef();
            }
            pSourceReader = _src.pSourceReader;
            if (pSourceReader) {
                pSourceReader->AddRef();
            }
            stream_base_time = _src.stream_base_time;
            last_barcode_time = _src.last_barcode_time;
            camera_status = _src.camera_status;
            init();
        }

        camera_control(container_control_base* _parent, int _id) : draw_control(_parent, _id)
        {
            pSource = nullptr;
            pSourceReader = nullptr;
            stream_base_time = 0;
            last_barcode_time = 0;
            camera_status = "Starting";
            init();
        }

        std::function<void(camera_control* _src)> camera_changed;

        virtual bool is_camera() { return true; }

        ring_buffer<sprinkle, 300> current_sprinkles;


        void select_camera(std::string _camera_name)
        {
            if (selected_camera_name.empty() or selected_camera_name != _camera_name) {
                selected_camera_name = _camera_name;
                stop();
            }
        }

        std::vector<std::string> get_cameras()
        {
            return available_cameras;
        }

        virtual void hardware_scan()
        {
            stop();
        }

        void add_sprinkles(sprinkle_buffer& new_sprinkles)
        {
            int num_new_sprinkles = new_sprinkles.get_size();

            for (int i = 0; i < num_new_sprinkles; i++)
            {
                sprinkle spr = new_sprinkles.get(i);
                current_sprinkles.put(spr);
            }
        }

        double get_detection_threshold() {
            return delta_boi.detection_threshold;
        }

        double get_activation_area_percentage() {
            return delta_boi.activation_area_percentage;
        }

        double get_detection_pulse() {
            return delta_boi.detection_pulse;
        }

        double get_detection_cooldown() {
            return delta_boi.detection_cooldown;
        }

        double set_detection_threshold(double _detection_threshold) {
            return delta_boi.detection_threshold = _detection_threshold;
        }

        double set_activation_area_percentage(double _activation_area_percentage) {
            return delta_boi.activation_area_percentage = _activation_area_percentage;
        }

        double set_detection_pulse(double _detection_pulse) {
            return delta_boi.detection_pulse = _detection_pulse;
        }

        double set_detection_cooldown(double _detection_cooldown) {
            return delta_boi.detection_cooldown = _detection_cooldown;
        }


        std::vector<movement_box> get_movement_boxes()
        {
            return delta_boi.get_movement_boxes();
        }

        ID2D1Bitmap* get_camera_image(ID2D1DeviceContext* _context)
        {
            return delta_boi.get_frame(_context);
        }

        HRESULT set_device_format(IMFMediaSource* _pSource, IMFMediaType* pType)
        {
            IMFPresentationDescriptor* pPD = NULL;
            IMFStreamDescriptor* pSD = NULL;
            IMFMediaTypeHandler* pHandler = NULL;

            HRESULT hr = _pSource->CreatePresentationDescriptor(&pPD);
            if (FAILED(hr))
            {
                goto done;
            }

            BOOL fSelected;
            hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = pSD->GetMediaTypeHandler(&pHandler);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = pHandler->SetCurrentMediaType(pType);

        done:
            if (pPD) {
                pPD->Release();
                pPD = nullptr;
            }
            if (pSD) {
                pSD->Release();
                pSD = nullptr;
            }
            if (pHandler) {
                pHandler->Release();
                pHandler = nullptr;
            }
            if (pType) {
                pType->Release();
                pType = nullptr;
            }
            return hr;
        }

        HRESULT find_device_format(IMFMediaSource* _pSource, IMFMediaType** ppType)
        {
            IMFPresentationDescriptor* pPD = NULL;
            IMFStreamDescriptor* pSD = NULL;
            IMFMediaTypeHandler* pHandler = NULL;
            IMFMediaType* pType = NULL;
            DWORD cTypes = 0;

            UINT32 max_feed_width = 0;
            UINT32 max_feed_height = 0;
            BOOL found_match = false;

            UINT32 feed_width;
            UINT32 feed_height;

            if (not ppType)
                return E_FAIL;

            *ppType = nullptr;

            HRESULT hr = _pSource->CreatePresentationDescriptor(&pPD);
            if (FAILED(hr))
            {
                goto done;
            }

            BOOL fSelected;
            hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = pSD->GetMediaTypeHandler(&pHandler);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = pHandler->GetMediaTypeCount(&cTypes);
            if (FAILED(hr))
            {
                goto done;
            }

            for (DWORD i = 0; i < cTypes and !found_match; i++)
            {
                hr = pHandler->GetMediaTypeByIndex(i, &pType);
                if (FAILED(hr))
                {
                    if (*ppType) {
                        (*ppType)->Release();
                        (*ppType) = nullptr;
                    }
                    goto done;
                }

                if (pType)
                {
                    GUID major_type, minor_type;
                    hr = pType->GetGUID(MF_MT_MAJOR_TYPE, &major_type);
                    if (SUCCEEDED(hr))
                    {
                        if (major_type == MFMediaType_Video) {

                            hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &feed_width, &feed_height);
                            if (SUCCEEDED(hr))
                            {
                                if (feed_width > max_feed_width and feed_height > max_feed_height and ppType)
                                {
                                    if (*ppType)
                                        (*ppType)->Release();

                                    *ppType = pType;
                                    (*ppType)->AddRef();

                                    max_feed_width = feed_width;
                                    max_feed_height = feed_height;

                                    if (feed_width >= 640) {
                                        found_match = true;
                                    }
                                }
                            }
                        }
                    }
                    pType->Release();
                }
            }


        done:
            if (pPD) {
                pPD->Release();
                pPD = nullptr;
            }
            if (pSD) {
                pSD->Release();
                pSD = nullptr;
            }
            if (pHandler) {
                pHandler->Release();
                pHandler = nullptr;
            }
            return hr;
        }

        virtual HRESULT start()
        {

            UINT32 count = 0;

            IMFAttributes* pConfig = NULL;
            IMFActivate** ppDevices = NULL;

            FILETIME ft;
            LARGE_INTEGER li;

            scope_lock lck(camera_access_lock);

            if (pSourceReader)
                return S_OK;

            // Create an attribute store to hold the search criteria.
            HRESULT hr = MFCreateAttributes(&pConfig, 1);

            // Request video capture devices.
            if (SUCCEEDED(hr))
            {
                hr = pConfig->SetGUID(
                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                    MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
                );
            }

            // Enumerate the devices,
            if (SUCCEEDED(hr))
            {
                hr = MFEnumDeviceSources(pConfig, &ppDevices, &count);
            }

            available_cameras.clear();

            // Create a media source for the first device in the list.
            if (SUCCEEDED(hr))
            {
                if (count > 0)
                {
                    int selected_device_index = -1;

                    for (int i = 0; i < count; i++) {
                        auto pdevice = ppDevices[i];

                        PWSTR deviceName = nullptr;
                        hr = MFGetAttributeString(pdevice, MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, &deviceName);
                        if (SUCCEEDED(hr) and deviceName) {
                            istring<256> tcamera_name;
                            std::string scamera_name;
                            tcamera_name = deviceName;
                            scamera_name = tcamera_name;
                            available_cameras.push_back(scamera_name);
                            if (selected_camera_name.empty()) {
                                selected_camera_name = scamera_name;
                            }
                            if (scamera_name == selected_camera_name) {
                                current_camera_name = selected_camera_name;
                                selected_device_index = i;
                            }
                            ::CoTaskMemFree(deviceName);
                        }
                    }

                    if (selected_device_index == -1)
                    {
                        selected_device_index = 0;
                        current_camera_name = available_cameras[0];
                        selected_camera_name = available_cameras[0];
                    }

                    if (camera_changed)
                        camera_changed(this);

                    auto pdevice = ppDevices[selected_device_index];

                    IMFMediaSource* newSource = nullptr;
                    IMFSourceReader* newSourceReader = nullptr;

                    hr = pdevice->ActivateObject(IID_PPV_ARGS(&newSource));
                    if (SUCCEEDED(hr) and newSource) {

                        ::GetSystemTimePreciseAsFileTime(&ft);
                        li.HighPart = ft.dwHighDateTime;
                        li.LowPart = ft.dwLowDateTime;
                        stream_base_time = li.QuadPart;

                        IMFMediaType* ptype = nullptr;
                        hr = find_device_format(newSource, &ptype);

                        if (SUCCEEDED(hr) and ptype)
                        {
                            hr = set_device_format(newSource, ptype);

                            if (SUCCEEDED(hr)) {
                                IMFAttributes* pAttributes = nullptr;

                                hr = MFCreateAttributes(
                                    &pAttributes,
                                    2
                                );

                                if (pAttributes)
                                {
                                    pAttributes->SetUINT32(MF_SOURCE_READER_DISCONNECT_MEDIASOURCE_ON_SHUTDOWN, FALSE);
                                }

                                if (pAttributes)
                                {
                                    pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
                                }

                                hr = MFCreateSourceReaderFromMediaSource(
                                    newSource,
                                    pAttributes,
                                    &newSourceReader
                                );

                                if (newSource and newSourceReader)
                                {
                                    hr = configure_decoder(newSourceReader, MF_SOURCE_READER_FIRST_VIDEO_STREAM);
                                    pSource = newSource;
                                    pSourceReader = newSourceReader;
                                }
                                else
                                {
                                    if (newSourceReader) {
                                        newSourceReader->Release();
                                    }
                                    if (newSource)
                                    {
                                        newSource->Shutdown();
                                        newSource->Release();
                                    }
                                }
                                return hr;
                            }
                        }
                    }
                }
                else
                {
                    camera_status = "No camera found";
                    system_monitoring_interface::global_mon->log_warning(camera_status);
                    hr = E_FAIL;
                }
            }
            else
            {
                camera_status = "No camera found";
                system_monitoring_interface::global_mon->log_warning(camera_status);
            }

            return hr;
        }

        std::string get_camera_name()
        {
            return current_camera_name;
        }

        virtual void arrange(rectangle _ctx)
        {
            draw_control::arrange(_ctx);
        }

        virtual void read_frame()
        {
            scope_lock lck(camera_access_lock);
            if (pSourceReader)
            {
                HRESULT hr = S_OK;
                IMFSample* pSample = NULL;

                DWORD streamIndex, flags;
                LONGLONG llTimeStamp, currentStamp;
                FILETIME ft;
                LARGE_INTEGER li;

                hr = pSourceReader->ReadSample(
                    MF_SOURCE_READER_ANY_STREAM,    // Stream index.
                    0,                              // Flags.
                    &streamIndex,                   // Receives the actual stream index. 
                    &flags,                         // Receives status flags.
                    &llTimeStamp,                   // Receives the time stamp.
                    &pSample                        // Receives the sample or NULL.
                );

                ::GetSystemTimePreciseAsFileTime(&ft);

                li.HighPart = ft.dwHighDateTime;
                li.LowPart = ft.dwLowDateTime;
                li.QuadPart -= stream_base_time;

                if (li.QuadPart > llTimeStamp) {
                    return;
                }

                if (FAILED(hr))
                {
                    if (hr == 0xc00d3ea2) {
                        camera_status = "Camera disconected";
                        system_monitoring_interface::global_mon->log_warning(camera_status);
                    }
                    else {
                        camera_status = "Camera error";
                        system_monitoring_interface::global_mon->log_warning(camera_status);
                    }
                    stop();
                    return;
                }

                if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
                {
                    return;
                }
                if (flags & MF_SOURCE_READERF_NEWSTREAM)
                {
                    system_monitoring_interface::global_mon->log_information("New camera stream");
                }
                if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
                {
                    system_monitoring_interface::global_mon->log_information("Native media type changed");
                }
                if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
                {
                    system_monitoring_interface::global_mon->log_information("Current media type changed");
                }
                if (flags & MF_SOURCE_READERF_STREAMTICK)
                {
                    ;
                }
                if (flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED)
                {
                    system_monitoring_interface::global_mon->log_information("Format changed");
                    hr = configure_decoder(pSourceReader, streamIndex);
                }

                if (pSample)
                {
                    IMFMediaType* pMediaType = nullptr;
                    UINT videoWidth = 0;
                    UINT videoHeight = 0;

                    if (SUCCEEDED(pSourceReader->GetCurrentMediaType(uint32_t(MF_SOURCE_READER_FIRST_VIDEO_STREAM), &pMediaType)))
                    {
                        MFVideoArea videoArea = {};
                        if (SUCCEEDED(pMediaType->GetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE, (uint8_t*)&videoArea, sizeof(MFVideoArea), nullptr)))
                        {
                            videoWidth = UINT(videoArea.Area.cx);
                            videoHeight = UINT(videoArea.Area.cy);
                        }
                        else
                        {
                            hr = MFGetAttributeSize(pMediaType, MF_MT_FRAME_SIZE, &videoWidth, &videoHeight);
                        }
                    }

                    IMFMediaBuffer* pMediaBuffer = nullptr;
                    IMF2DBuffer* p2dBuffer = nullptr;

                    hr = pSample->ConvertToContiguousBuffer(&pMediaBuffer);
                    if (SUCCEEDED(hr) and pMediaBuffer) {
                        camera_status = "";
                        hr = pMediaBuffer->QueryInterface(&p2dBuffer);
                        if (SUCCEEDED(hr) and p2dBuffer) {
                            BYTE* byte_start = nullptr;
                            long pitch = 0;
                            // here is where we can fill the MAT for OpenCV, in addition to
                            hr = p2dBuffer->Lock2D(&byte_start, &pitch);
                            if (SUCCEEDED(hr) and byte_start and pitch)
                            {
                                int64_t diff = (li.QuadPart - last_barcode_time) / 10000000;

                                bgra32_pixel* px = (bgra32_pixel*)byte_start;
                                delta_boi.next_frame(px, videoWidth, videoHeight, videoWidth);

                                p2dBuffer->Unlock2D();
                                p2dBuffer->Release();
                                p2dBuffer = nullptr;
                            }
                        }
                        pMediaBuffer->Release();
                        pMediaBuffer = nullptr;
                    }

                    pSample->Release();
                    pSample = nullptr;
                }
            }
            else if (camera_check_timer.check(2.0))
            {
                camera_status = "Checking for camera";
                start();
                if (auto phost = host.lock()) {
                    ::PostMessage(phost->getMainWindow(), WM_CORONA_RESET, 0, 0);
                }
            }
        }

        virtual void stop()
        {
            scope_lock lck(camera_access_lock);

            auto tempReader = pSourceReader;
            auto tempSource = pSource;

            pSourceReader = nullptr;
            pSource = nullptr;

            if (tempReader) {
                tempReader->Release();
            }

            if (tempSource) {
                tempSource->Shutdown();
                tempSource->Release();
            }
        }

        virtual void destroy()
        {
            stop();
        }

        virtual ~camera_control()
        {
            destroy();
        }

        virtual void on_update(double _time)
        {
            read_frame();
            for (auto child : children) {
                child->on_update(_time);
            }
        }
    };

    class camera_view_control :
        public draw_control
    {

        void init();

    public:

        int64_t frame_counter;

        int	camera_control_id;

        camera_view_control();
        camera_view_control(const camera_view_control& _src);
        camera_view_control(container_control_base* _parent, int _id);
        virtual ~camera_view_control();
    };

    using cell_json_size = std::function<point(draw_control* _parent, int _index, json _data, rectangle _bounds)>;
    using cell_json_draw = std::function<void(std::shared_ptr<direct2dContext>& _src, draw_control* _parent, int _index, json _data, rectangle _bounds)>;
    using cell_json_assets = std::function<void(std::shared_ptr<direct2dContext>& _src, draw_control* _parent, rectangle _bounds)>;

    class array_data_source
    {
    public:
        json				data;
        cell_json_assets	assets;
        cell_json_draw		draw_item;
        cell_json_size		size_item;
    };

    class grid_control : public draw_control
    {
        solidBrushRequest	border_brush;
    public:
        grid_control(const grid_control& _src) : draw_control(_src)
        {
            border_brush = _src.border_brush;
        }
        grid_control(container_control_base* _parent, int _id) : draw_control(_parent, _id)
        {
            ;
        }

        virtual ~grid_control()
        {
            ;
        }

    };

    const int IDC_CHART_BASE = 1000000;
    const int IDC_CHART_PROGRAM = 1 + IDC_CHART_BASE;
    const int IDC_CHART_BAR = 2 + IDC_CHART_BASE;
    const int IDC_CHART_LINE = 3 + IDC_CHART_BASE;
    const int IDC_CHART_PIE = 4 + IDC_CHART_BASE;
    const int IDC_CHART_SCATTER = 5 + IDC_CHART_BASE;
    const int IDC_CHART_BUBBLE = 6 + IDC_CHART_BASE;
    const int IDC_CHART_TREE_BOX = 7 + IDC_CHART_BASE;
    const int IDC_CHART_TREE_MAP = 8 + IDC_CHART_BASE;

    class chart_control : public draw_control
    {
        void init();
        solidBrushRequest	border_brush;

        /*

        The big thing for the design here is to just be the mainframe sleazes when it comes to handling loads of options.
        For the type of chart, you can make in the menu into it.

        */

    public:

        int chart_type;
        table_data  data;
        std::string color_series;
        std::string sseries1;
        std::string sseries2;
        std::string sseries3;
        std::string sseries4;

        chart_control(const chart_control& _src) : draw_control(_src)
        {
            border_brush = _src.border_brush;
            chart_type = _src.chart_type;
            data = _src.data;
            color_series = _src.color_series;
            sseries1 = _src.sseries1;
            sseries2 = _src.sseries2;
            sseries3 = _src.sseries3;
            sseries4 = _src.sseries4;
        }

        chart_control(container_control_base* _parent, int _id) : draw_control(_parent, _id)
        {
            ;
        }

        virtual ~chart_control()
        {
            ;
        }

    };


    class slide_control : public draw_control
    {
        void init();
        solidBrushRequest	border_brush;

    public:

        int source_object_id;

        slide_control(container_control_base* _parent, int _id) : draw_control(_parent, _id) { ; }
        virtual ~slide_control() { ; }

    };

    class gradient_button_control : public draw_control
    {
    public:

        linearGradientBrushRequest buttonFaceNormal;
        linearGradientBrushRequest buttonFaceDown;
        linearGradientBrushRequest buttonFaceOver;
        radialGradientBrushRequest buttonBackLight;

        solidBrushRequest foregroundNormal;
        solidBrushRequest foregroundOver;
        solidBrushRequest foregroundDown;

        gradient_button_control()
        {
            ;
        }

        gradient_button_control(const gradient_button_control& _src) : draw_control(_src)
        {
            buttonFaceNormal = _src.buttonFaceNormal;
            buttonFaceDown = _src.buttonFaceDown;
            buttonFaceOver = _src.buttonFaceOver;
            buttonBackLight = _src.buttonBackLight;

            foregroundNormal = _src.foregroundNormal;
            foregroundOver = _src.foregroundOver;
            foregroundDown = _src.foregroundDown;
        }

        gradient_button_control(container_control_base* _parent, int _id, std::string _base_name) : draw_control(_parent, _id)
        {
            buttonFaceNormal.name = _base_name + "_face_normal";
            buttonFaceOver.name = _base_name + "_face_over";
            buttonFaceDown.name = _base_name + "_face_down";

            foregroundNormal.name = _base_name + "_fore_normal";
            foregroundOver.name = _base_name + "_fore_over";
            foregroundDown.name = _base_name + "_fore_down";

            std::string face_step = "#79726eff";
            std::string light_step = "#adb3b7ff";
            std::string dark_step = "#3b413cff";
            std::string darkest_step = "#353535ff";

            buttonFaceNormal.gradientStops = {
                { toColor(dark_step), 0.0 },
                { toColor(light_step), 0.8 },
                { toColor(dark_step), 1.0 },
            };

            buttonFaceOver.gradientStops = {
                { toColor(dark_step), 0.0 },
                { toColor(light_step), 0.8 },
                { toColor(dark_step), 1.0 },
            };

            buttonFaceDown.gradientStops = {
                { toColor(dark_step), 0.0 },
                { toColor(light_step), 0.9 },
                { toColor(dark_step), 1.0 },
            };

            buttonBackLight.gradientStops = {
                { toColor("#00000000"), 0.0 },
                { toColor("#C0C0C040"), 0.9 },
                { toColor("#E0F0E0FF"), 1.0 }
            };

            foregroundNormal.brushColor = toColor("#C5C6CA");
            foregroundOver.brushColor = toColor("#F5F6FA");
            foregroundDown.brushColor = toColor("#FFD700");
        }

        virtual ~gradient_button_control()
        {
            ;
        }

        virtual void draw_button(std::shared_ptr<direct2dContext>& _context, std::function<void(std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape)
        {
            if (auto pwindow = window.lock())
            {
                if (auto phost = host.lock()) {
                    auto draw_bounds = inner_bounds;

                    buttonFaceNormal.start.x = draw_bounds.x + draw_bounds.w / 2;
                    buttonFaceNormal.start.y = draw_bounds.y;
                    buttonFaceNormal.stop.x = draw_bounds.x + draw_bounds.w / 2;
                    buttonFaceNormal.stop.y = draw_bounds.y + draw_bounds.h;

                    buttonFaceDown.start.x = draw_bounds.x + draw_bounds.w / 2;
                    buttonFaceDown.start.y = draw_bounds.y;
                    buttonFaceDown.stop.x = draw_bounds.x + draw_bounds.w / 2;
                    buttonFaceDown.stop.y = draw_bounds.y + draw_bounds.h;

                    buttonFaceOver.start.x = draw_bounds.x + draw_bounds.w / 2;
                    buttonFaceOver.start.y = draw_bounds.y;
                    buttonFaceOver.stop.x = draw_bounds.x + draw_bounds.w / 2;
                    buttonFaceOver.stop.y = draw_bounds.y + draw_bounds.h;

                    buttonBackLight.center = rectangle_math::center(draw_bounds);
                    buttonBackLight.offset = {};
                    buttonBackLight.radiusX = draw_bounds.w / 2.0;
                    buttonBackLight.radiusY = draw_bounds.h / 2.0;

                    _context->setLinearGradientBrush(&this->buttonFaceNormal);
                    _context->setLinearGradientBrush(&this->buttonFaceDown);
                    _context->setLinearGradientBrush(&this->buttonFaceOver);
                    _context->setRadialGradientBrush(&this->buttonBackLight);
                    _context->setSolidColorBrush(&this->foregroundNormal);
                    _context->setSolidColorBrush(&this->foregroundDown);
                    _context->setSolidColorBrush(&this->foregroundOver);


                    auto& context = _context;

                    if (mouse_left_down.value())
                    {
                        _context->drawRectangle(&draw_bounds, "", 0.0, buttonFaceDown.name);
                        auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 8 });
                        draw_shape(_context, this, &face_bounds, &foregroundDown);
                    }
                    else if (mouse_over.value())
                    {
                        _context->drawRectangle(&draw_bounds, "", 0.0, buttonFaceOver.name);
                        auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 16 });
                        draw_shape(_context, this, &face_bounds, &foregroundOver);
                    }
                    else
                    {
                        _context->drawRectangle(&draw_bounds, "", 0.0, buttonFaceNormal.name);
                        auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 16 });
                        draw_shape(_context, this, &face_bounds, &foregroundNormal);
                    }
                }
            }
        }


    };

    class minimize_button_control : public gradient_button_control
    {
    public:

        minimize_button_control() { ; }
        minimize_button_control(const minimize_button_control& _src) : gradient_button_control(_src) { ; }
        minimize_button_control(container_control_base* _parent, int _id);

        virtual ~minimize_button_control();
        virtual LRESULT get_nchittest() {
            return HTCLIENT; // we lie here 
        }

        virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

    };

    class maximize_button_control : public gradient_button_control
    {
    public:

        maximize_button_control() { ; }
        maximize_button_control(const maximize_button_control& _src) : gradient_button_control(_src) { ; }
        maximize_button_control(container_control_base* _parent, int _id);

        virtual ~maximize_button_control();
        virtual LRESULT get_nchittest() {
            return HTCLIENT;// we lie here 
        }

        virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

    };

    class close_button_control : public gradient_button_control
    {
    public:

        close_button_control() { ; }
        close_button_control(const close_button_control& _src) : gradient_button_control(_src) { ; }

        close_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "close")
        {
            auto ctrl = this;

            on_draw = [this](std::shared_ptr<direct2dContext>& _context, control_base* _item)
                {
                    if (auto pwindow = window.lock())
                    {
                        if (auto phost = host.lock()) {
                            auto draw_bounds = inner_bounds;

                            std::function<void(std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;

                            point shape_origin;
                            point* porigin = &shape_origin;

                            draw_shape = [this, porigin](std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground) {
                                point start, stop;
                                start.x = _bounds->x;
                                start.y = _bounds->y;
                                stop.x = _bounds->right();
                                stop.y = _bounds->bottom();
                                _context->drawLine(&start, &stop, _foreground->name, 4);
                                start.x = _bounds->right();
                                start.y = _bounds->y;
                                stop.x = _bounds->x;
                                stop.y = _bounds->bottom();
                                _context->drawLine(&start, &stop, _foreground->name, 4);
                                };

                            draw_button(_context, draw_shape);
                        }
                    }
                };
        }

        virtual LRESULT get_nchittest() {
            return HTCLIENT;// we lie here 
        }


        void on_subscribe(presentation_base* _presentation, page_base* _page)
        {
            _page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
                {
                    _presentation->close_window();
                });
        }

        virtual ~close_button_control()
        {
            ;
        }

    };

    class menu_button_control : public gradient_button_control
    {
    public:

        menu_item menu;

        menu_button_control()
        {
            ;
        }
        menu_button_control(const menu_button_control& _src) : gradient_button_control(_src) {
            menu = _src.menu;
        }
        menu_button_control(container_control_base* _parent, int _id);
        virtual ~menu_button_control() { ; }

        virtual void on_subscribe(presentation_base* _presentation, page_base* _page);
    };

    class tab_button_control : public gradient_button_control
    {
    public:

        std::string			text;
        solidBrushRequest	text_idle_brush;
        textStyleRequest	text_style;
        textStyleRequest	selected_text_style;
        double				icon_width;
        int* active_id;
        std::function<void(tab_button_control& _tb)> tab_selected;

        tab_button_control() : active_id(nullptr)
        {
            init();
        }

        tab_button_control(const tab_button_control& _src) : gradient_button_control(_src) {
            init();
            text = _src.text;
            text_idle_brush = _src.text_idle_brush;
            text_style = _src.text_style;
            icon_width = _src.icon_width;
            active_id = _src.active_id;
        }
        tab_button_control(container_control_base* _parent, int _id);
        virtual ~tab_button_control() { ; }

        void init();
        virtual double get_font_size() { return text_style.fontSize; }
        tab_button_control& set_text(std::string _text);
        tab_button_control& set_text_fill(solidBrushRequest _brushFill);
        tab_button_control& set_text_fill(std::string _color);
        tab_button_control& set_text_style(std::string _font_name, int _font_size, bool _bold = false, bool _underline = false, bool _italic = false, bool _strike_through = false);
        tab_button_control& set_text_style(textStyleRequest request);

        virtual void on_subscribe(presentation_base* _presentation, page_base* _page);

        virtual void draw_button(std::shared_ptr<direct2dContext>& _context, std::function<void(std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape)
        {
            if (auto pwindow = window.lock())
            {
                if (auto phost = host.lock()) {
                    auto draw_bounds = inner_bounds;

                    auto& context = _context; // pwindow->getContext();

                    if (mouse_left_down.value() or *active_id == id)
                    {
                        _context->drawRectangle(&draw_bounds, "", 0.0, buttonFaceDown.name);
                        auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 8 });
                        draw_shape(_context, this, &face_bounds, &foregroundDown);
                    }
                    else if (mouse_over.value())
                    {
                        _context->drawRectangle(&draw_bounds, "", 0.0, buttonFaceOver.name);
                        auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 8 });
                        draw_shape(_context, this, &face_bounds, &foregroundOver);
                    }
                    else
                    {
                        _context->drawRectangle(&draw_bounds, "", 0.0, buttonFaceNormal.name);
                        auto face_bounds = rectangle_math::deflate(draw_bounds, { 8, 8, 8, 8 });
                        draw_shape(_context, this, &face_bounds, &foregroundNormal);
                    }
                }
            }
        }

    };

    class image_control :
        public draw_control
    {

        enum image_modes {
            no_image,
            use_filename,
            use_control_id,
            use_resource_id
        };

        bitmapInstanceDto instance;

        image_modes		image_mode;

        std::string		image_filename;
        int				image_control_id;
        DWORD			image_resource_id;
        std::string		image_name;

        void init();

    public:
        image_control();
        image_control(const image_control& _src) = default;
        image_control(container_control_base* _parent, int _id);
        image_control(container_control_base* _parent, int _id, std::string _file_name);
        image_control(container_control_base* _parent, int _id, int _source_control_id);
        virtual ~image_control();

        virtual void get_json(json& _dest)
        {
            draw_control::get_json(_dest);
            _dest.put_member("image_filename", image_filename);
        }
        virtual void put_json(json& _src)
        {
            draw_control::put_json(_src);
            load_from_file(_src["image_filename"]);
            init();
        }

        void load_from_file(std::string _name);
        void load_from_resource(DWORD _resource_id);
        void load_from_control(int _control_id);
    };



    image_control::image_control()
    {
        init();
    }

    image_control::image_control(container_control_base* _parent, int _id)
        : draw_control(_parent, _id)
    {
        init();
    }

    image_control::image_control(container_control_base* _parent, int _id, std::string _filename) : draw_control(_parent, _id)
    {
        init();
        load_from_file(_filename);
    }

    image_control::image_control(container_control_base* _parent, int _id, int _source_control_id) : draw_control(_parent, _id)
    {
        init();
        load_from_control(_source_control_id);
    }

    void image_control::load_from_file(std::string _name)
    {
        image_mode = image_modes::use_filename;
        image_filename = _name;
        instance.bitmapName = std::format("bitmap_file_{0}", _name);
    }

    void image_control::load_from_resource(DWORD _resource_id)
    {
        image_mode = image_modes::use_resource_id;
        image_resource_id = _resource_id;
        instance.bitmapName = std::format("bitmap_resource_{0}", _resource_id);
    }

    void image_control::load_from_control(int _control_id)
    {
        image_mode = image_modes::use_control_id;
        image_control_id = _control_id;
        instance.bitmapName = std::format("bitmap_control_{0}_{1}", id, _control_id);
    }

    void image_control::init()
    {
        set_origin(0.0_px, 0.0_px);
        set_size(50.0_px, 50.0_px);

        on_create = [this](std::shared_ptr<direct2dContext>& _context, draw_control* _src)
            {
                if (auto pwindow = this->window.lock())
                {
                    auto& context = _context;

                    solidBrushRequest sbr;
                    sbr.brushColor = toColor("FFFF00");
                    sbr.name = "image_control_test";
                    context->setSolidColorBrush(&sbr);

                    switch (image_mode) {
                    case image_modes::use_control_id:
                        break;
                    case image_modes::use_resource_id:
                    {
                        bitmapRequest request = {};
                        request.resource_id = image_resource_id;
                        request.name = instance.bitmapName;
                        request.cropEnabled = false;
                        point pt = { inner_bounds.w, inner_bounds.h };
                        request.sizes.push_back(pt);
                        context->setBitmap(&request);
                        break;
                    }
                    break;
                    case image_modes::use_filename:
                    {
                        if (image_filename.size() == 0)
                            throw std::logic_error("Missing file name for image");
                        bitmapRequest request = {};
                        request.filename = image_filename;
                        request.name = instance.bitmapName;
                        request.cropEnabled = false;
                        point pt = { inner_bounds.w, inner_bounds.h };
                        request.sizes.push_back(pt);
                        context->setBitmap(&request);
                        auto szfound = std::begin(request.sizes);
                        if (szfound != std::end(request.sizes)) {
                            instance.width = request.sizes.begin()->x;
                            instance.height = request.sizes.begin()->y;
                        }
                        else
                        {
                            instance.width = 0;
                            instance.height = 0;
                        }
                        break;
                    }
                    }
                }
            };

        on_draw = [this](std::shared_ptr<direct2dContext>& _context, draw_control* _src) {
            if (auto pwindow = this->window.lock())
            {
                if (auto phost = host.lock()) {
                    auto draw_bounds = inner_bounds;


                    if (image_mode == image_modes::use_control_id)
                    {

                    }
                    else
                    {

                        instance.copyId = 0;
                        instance.selected = false;
                        instance.x = draw_bounds.x;
                        instance.y = draw_bounds.y;
                        instance.width = draw_bounds.w;
                        instance.height = draw_bounds.h;
                        instance.alpha = 1.0;

                        auto& context = _context; // window->getContext();

                        context->drawBitmap(&instance);
                    }
                }
            }
            };
    }

    image_control::~image_control()
    {
        ;
    }

    camera_view_control::camera_view_control()
    {
        camera_control_id = -1;
        init();
    }

    camera_view_control::camera_view_control(const camera_view_control& _src) :
        draw_control(_src),
        camera_control_id(_src.camera_control_id)
    {
        init();
    }

    camera_view_control::camera_view_control(container_control_base* _parent, int _id) : draw_control(_parent, _id)
    {
        init();
    }

    void camera_view_control::init()
    {
        frame_counter = 0;
        set_origin(0.0_px, 0.0_px);
        set_size(50.0_px, 50.0_px);

        on_create = [this](std::shared_ptr<direct2dContext>& _context, draw_control* _src)
            {
            };

        on_draw = [this](std::shared_ptr<direct2dContext>& _context, draw_control* _src) {
            frame_counter++;
               if (auto phost = host.lock()) {
                    auto draw_bounds = inner_bounds;

                    draw_bounds.x = inner_bounds.x;
                    draw_bounds.y = inner_bounds.y;

                    control_base* camb = find(camera_control_id);
                    if (camb) {
                        camera_control* cam = dynamic_cast<camera_control*>(camb);
                        if (cam) {
                            auto* dc = _context->getDeviceContext();

                            auto* bm = cam->get_camera_image(dc);
                            if (bm)
                            {
                                auto movement_boxes = cam->get_movement_boxes();

                                rectangle dest_box;
                                dest_box.x = draw_bounds.x;
                                dest_box.y = draw_bounds.y;
                                dest_box.w = (draw_bounds.w - 32) / 2;
                                if (dest_box.w < 0)
                                    return;

                                for (int i = 0; i < movement_boxes.size() and i < 4; i++)
                                {
                                    auto source_box = movement_boxes[i];
                                    dest_box.h = dest_box.w / source_box.area.w * source_box.area.h;

                                    D2D1_RECT_F source_rect;
                                    D2D1_RECT_F dest_rect;

                                    source_rect.left = source_box.area.x;
                                    source_rect.top = source_box.area.y;
                                    source_rect.right = source_box.area.right();
                                    source_rect.bottom = source_box.area.bottom();

                                    dest_rect.left = dest_box.x;
                                    dest_rect.top = dest_box.y;
                                    dest_rect.right = dest_box.right();
                                    dest_rect.bottom = dest_box.bottom();

                                    dc->DrawBitmap(bm, &dest_rect, 1.0, D2D1_BITMAP_INTERPOLATION_MODE::D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &source_rect);

                                    dest_box.x += dest_box.w + padding_amount.x;

                                    if (dest_box.right() > draw_bounds.right())
                                    {
                                        dest_box.x = draw_bounds.x;
                                        dest_box.y += (draw_bounds.h - 32) / 2 + 8;
                                    }
                                }

                                bm->Release();
                            }
                        }
                    }
                }
            };
    }

    camera_view_control::~camera_view_control()
    {
        ;
    }

    menu_button_control::menu_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "menu")
    {
        auto ctrl = this;

        on_draw = [this](std::shared_ptr<direct2dContext>& _context, control_base* _item)
            {
                if (auto pwindow = window.lock())
                {
                    if (auto phost = host.lock()) {
                        auto draw_bounds = inner_bounds;

                        std::function<void(std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;


                        point shape_origin;
                        point* porigin = &shape_origin;

                        draw_shape = [this, porigin](std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground) {

                            point start;
                            point stop;

                            start.x = _bounds->x;
                            start.y = _bounds->y + _bounds->h / 2.0;
                            stop.x = _bounds->right();
                            stop.y = _bounds->y + _bounds->h / 2.0;

                            _context->drawLine(&start, &stop, _foreground->name, 4);

                            pathImmediateDto pid;
                            porigin->x = _bounds->x;
                            porigin->y = _bounds->y;
                            porigin->z = 0;
                            pid.path.addLineTo(_bounds->x, _bounds->y);
                            pid.path.addLineTo(_bounds->right(), _bounds->y);
                            pid.path.addLineTo(_bounds->right(), _bounds->bottom());
                            pid.path.addLineTo(_bounds->x, _bounds->bottom());
                            pid.path.addLineTo(_bounds->x, _bounds->y);
                            pid.position = *porigin;
                            pid.rotation = 0;
                            pid.strokeWidth = 4;
                            pid.borderBrushName = _foreground->name;
                            pid.closed = true;
                            _context->drawPath(&pid);
                            };

                        draw_button(_context, draw_shape);

                    }
                }
            };
    }

    void menu_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
    {
        menu.subscribe(_presentation, _page);
        _page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
            {
                _presentation->open_menu(this, this->menu);
            });
    }

    minimize_button_control::minimize_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "minimize")
    {
        auto ctrl = this;

        on_draw = [this](std::shared_ptr<direct2dContext>& _context, control_base* _item)
            {
                if (auto pwindow = window.lock())
                {
                    if (auto phost = host.lock()) {
                        auto draw_bounds = inner_bounds;

                        std::function<void(std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;


                        point shape_origin;
                        point* porigin = &shape_origin;

                        draw_shape = [this, porigin](std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground) {
                            pathImmediateDto pid;
                            porigin->x = _bounds->x;
                            porigin->y = _bounds->y;
                            porigin->z = 0;
                            pid.path.addLineTo(_bounds->x, _bounds->y);
                            pid.path.addLineTo(_bounds->x + _bounds->w / 2.0, _bounds->bottom());
                            pid.path.addLineTo(_bounds->right(), _bounds->y);
                            pid.position = *porigin;
                            pid.rotation = 0;
                            pid.strokeWidth = 4;
                            pid.borderBrushName = _foreground->name;
                            pid.closed = true;
                            _context->drawPath(&pid);
                            };

                        draw_button(_context, draw_shape);

                    }
                }
            };
    }

    void minimize_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
    {
        _page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
            {
                _presentation->minimize_window();
            });
    }

    minimize_button_control::~minimize_button_control()
    {
        ;
    }

    maximize_button_control::maximize_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "maximize")
    {
        auto ctrl = this;

        on_draw = [this](std::shared_ptr<direct2dContext>& _context, control_base* _item)
            {
                if (auto pwindow = window.lock())
                {
                    if (auto phost = host.lock()) {
                        auto draw_bounds = inner_bounds;

                        std::function<void(std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;


                        point shape_origin;
                        point* porigin = &shape_origin;

                        draw_shape = [this, porigin](std::shared_ptr<direct2dContext> _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground) {
                            pathImmediateDto pid;
                            porigin->x = _bounds->x;
                            porigin->y = _bounds->y;
                            porigin->z = 0;
                            pid.path.addLineTo(_bounds->x, _bounds->bottom());
                            pid.path.addLineTo(_bounds->x + _bounds->w / 2.0, _bounds->y);
                            pid.path.addLineTo(_bounds->right(), _bounds->bottom());
                            pid.position = *porigin;
                            pid.rotation = 0;
                            pid.strokeWidth = 4;
                            pid.borderBrushName = _foreground->name;
                            pid.closed = true;
                            _context->drawPath(&pid);
                            };

                        draw_button(_context, draw_shape);

                    }
                }
            };
    }

    void maximize_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
    {
        _page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
            {
                _presentation->restore_window();
            });
    }

    maximize_button_control::~maximize_button_control()
    {
        ;
    }


    tab_button_control::tab_button_control(container_control_base* _parent, int _id) : gradient_button_control(_parent, _id, "menu")
    {
        init();
    }

    void tab_button_control::on_subscribe(presentation_base* _presentation, page_base* _page)
    {
        _page->on_mouse_left_click(this, [this, _presentation, _page](mouse_left_click_event evt)
            {
                if (active_id) {
                    *active_id = id;
                }
                if (tab_selected)
                {
                    tab_selected(*this);
                }
            });
    }

    void tab_button_control::init()
    {
        set_origin(0.0_px, 0.0_px);
        set_size(200.0_px, 1.0_container);
        icon_width = 8;

        auto st = presentation_style_factory::get_current()->get_style();

        text_style = {};
        text_style.name = "tab_text_style";
        text_style.fontName = st->PrimaryFont;
        text_style.fontSize = 14;
        text_style.bold = false;
        text_style.italics = false;
        text_style.underline = false;
        text_style.strike_through = false;
        text_style.horizontal_align = st->PrimaryAlignment;
        text_style.vertical_align = visual_alignment::align_center;
        text_style.wrap_text = true;
        text_style.font_stretch = DWRITE_FONT_STRETCH_NORMAL;

        auto ctrl = this;

        on_create = [this](std::shared_ptr<direct2dContext>& _context, draw_control* _src)
            {
                if (auto pwindow = this->window.lock())
                {
                    _context->setSolidColorBrush(&this->text_idle_brush);
                    _context->setTextStyle(&this->text_style);
                }
            };

        on_draw = [this](std::shared_ptr<direct2dContext>& _context, control_base* _item)
            {
                if (auto pwindow = window.lock())
                {
                    if (auto phost = host.lock()) {
                        auto draw_bounds = inner_bounds;

                        std::function<void(std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground)> draw_shape;


                        point shape_origin;
                        point* porigin = &shape_origin;

                        auto& context = _context; // pwindow->getContext();

                        draw_shape = [this, porigin](std::shared_ptr<direct2dContext>& _context, gradient_button_control* _src, rectangle* _bounds, solidBrushRequest* _foreground) {

                            rectangle icon_bounds = *_bounds;

                            if (active_id and *active_id == id) {
                                point start;
                                point stop;
                                start.x = icon_bounds.x + icon_width / 2;
                                start.y = icon_bounds.y;
                                stop.x = start.x;
                                stop.y = icon_bounds.y + icon_bounds.h;
                                //								pcontext->drawLine(&start, &stop, _foreground->name, 4);
                                _bounds->y -= 4;
                            }
                            else
                            {
                                point center;
                                point radius;
                                center.x = icon_bounds.x + icon_width / 2;
                                center.y = icon_bounds.y + icon_bounds.h / 2;
                                radius.x = icon_width / 2;
                                radius.y = icon_bounds.h / 2;
                                //							pcontext->drawEllipse(&center, &radius, _foreground->name, 4, nullptr);
                                _bounds->y -= 8;
                            }

                            _bounds->x += icon_width + 4;
                            _bounds->w -= icon_width + 4;

                            _context->drawText(text.c_str(), _bounds, this->text_style.name, _foreground->name);

                            };

                        draw_button(_context, draw_shape);
                    }
                }
            };
    }

    tab_button_control& tab_button_control::set_text(std::string _text)
    {
        text = _text;
        return *this;
    }

    tab_button_control& tab_button_control::set_text_fill(solidBrushRequest _brushFill)
    {
        text_idle_brush = _brushFill;
        return *this;
    }

    tab_button_control& tab_button_control::set_text_fill(std::string _color)
    {
        text_idle_brush.name = typeid(*this).name();
        text_idle_brush.brushColor = toColor(_color);
        return *this;
    }

    tab_button_control& tab_button_control::set_text_style(std::string _font_name, int _font_size, bool _bold, bool _underline, bool _italic, bool _strike_through)
    {
        text_style.name = typeid(*this).name();
        text_style.fontName = _font_name;
        text_style.fontSize = _font_size;
        text_style.bold = _bold;
        text_style.underline = _underline;
        text_style.italics = _italic;
        text_style.strike_through = _strike_through;
        return *this;
    }

    tab_button_control& tab_button_control::set_text_style(textStyleRequest request)
    {
        text_style = request;
        return *this;
    }
}

#endif
