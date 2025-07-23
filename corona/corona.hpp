/*
* 
CORONA 
C++ Low Code Performance Applications for Windows SDK
by Todd Bandrowky
(c) 2024+ All Rights Reserved

MIT License

About this File
This is the main header file.
It was structured to facilitate C++ modules but there 
is much work to be done, and at the time, there were issues
with the compiler.

Notes

For Future Consideration

*/


#ifndef CORONA_H
#define CORONA_H

#include "corona-windows-all.h"

#include <string>
#include <bit>
#include <cstring>
#include <cstdlib>
#include <compare>
#include <tuple>
#include <iostream>
#include <memory>
#include <exception>
#include <stdexcept>
#include <format>
#include <map>
#include <vector>
#include <random>
#include <stack>
#include <concepts>
#include <functional>
#include <algorithm>
#include <list>
#include <coroutine>
#include <initializer_list>
#include <filesystem>
#include <atomic>
#include <fstream>
#include <sstream>
#include <numeric>
#include <thread>
#include <regex>
#include <queue>
#include <ranges>

#include <io.h>
#include <fcntl.h>
#include <cmath>
#include <numbers>
#include <cctype>
#include <conio.h>
#include <direct.h>
#include <sql.h>
#include <sqlext.h>

// these are all miscellaneous utility classes and the ever abused core constants

#include <ciso646>

namespace corona {
	void log_warning(const std::string &_src);
}

#include "corona-publicity-stunts.hpp"

#include "corona-base64.hpp"
#include "corona-constants.hpp"
#include "corona-function-bag.hpp"
#include "corona-wchart_convert.hpp"
#include "corona-utility.hpp"
#include "corona-read_all_string.hpp"

// these are for a binary system of storing elements as opposed to json

#include "corona-store_box.hpp"
#include "corona-time_box.hpp"
#include "corona-iarray.hpp"
#include "corona-assert_if.hpp"
#include "corona-collection_id_box.hpp"
#include "corona-string_box.hpp"
#include "corona-float_box.hpp"
#include "corona-int_box.hpp"
#include "corona-midi_box.hpp"
#include "corona-object_id_box.hpp"

#include "corona-system-monitor-bus.hpp"
#include "corona-test.hpp"

// these are the same, but are also hella useful in the UI.
// a layout box is where Corona UI does element positioning.

#include "corona-point_box.hpp"
#include "corona-wave_box.hpp"
#include "corona-rectangle_box.hpp"
#include "corona-layout_rect_box.hpp"
#include "corona-image_box.hpp"

// implementation of http statuses

#include "corona-messages.hpp"
#include "corona-http-messages.hpp"

// generalized json representation used as lingua fraca throughout the bus

#include "corona-json.hpp"
#include "corona-json-serialization.hpp"
#include "corona-locks.hpp"
#include "corona-queue.hpp"
#include "corona-function.hpp"
#include "corona-file.hpp"
#include "corona-file-block.hpp"
#include "corona-directory.hpp"
#include "corona-application.hpp"
#include "corona-visual.hpp"
#include "corona-control-data.hpp"
#include "corona-interfaces.hpp"
#include "corona-comm-bus-interface.hpp"
#include "corona-express-record.hpp"

// these are general structures and are used for the binary saving stuff, not
// folded into the bus just yet.

#include "corona-filterable_iterator.hpp"
#include "corona-table.hpp"
#include "corona-list_box.hpp"
#include "corona-skip_list.hpp"
#include "corona-sorted_index.hpp"

#include "corona-grouping.hpp"
#include "corona-jfield.hpp"
#include "corona-jvalue.hpp"
#include "corona-jobject.hpp"
#include "corona-jdatabase_server.hpp"

// these are for the ui. data transfer is just about empty and was
// envisioned as the place for all the dtos, but, matters evolved and 
// they are all more placed in their own domains.  although, there's a right
// bit of stuff in corona-visual for brushes.
// and, the directx stuff is actually, for all the goings on under the hood.
// theoretically we can make this run on the server side as well.

#include "corona-datatransfer.hpp"
#include "corona-directxdevices.hpp"
#include "corona-directxadapterbase.hpp"
#include "corona-direct2dcontextbase.hpp"
#include "corona-bitmap_filters.hpp"
#include "corona-motion-detect.hpp"

// I mentioned data transfer previously, and here you can see things like this
// are now more spread out.  These bridge direct2d assets as the context
// can deal with a surface loss or moving across multiple monitors and 
// all of that under the hood awesomeness.

#include "corona-direct2dresources.hpp"
#include "corona-bitmaps.hpp"
#include "corona-brushes.hpp"
#include "corona-paths.hpp"
#include "corona-textstyles.hpp"

// this is a general http client.  It's just a thin wrapper around windows http client
#include "corona-httpclient.hpp"

// these implement some of the abstractions needed to build the ui
// here we have a device context, which wraps the direct 2d native context
// and handles things like being able to use brushes by name

#include "corona-direct2dcontext.hpp"
#include "corona-directxadapter.hpp"
#include "corona-direct2dwindow.hpp"
#include "corona-controller.hpp"
#include "corona-application_base.hpp"

// cryptography implements windows crypto services, and
// is used for both hashing and encryption.
#include "corona-cryptography.hpp"

// query implements a linq like predicate object graph for everywhere
#include "corona-query.hpp"

// cheesy infix parser to be used 
#include "corona-expressions.hpp"

// the data block allows for serialization and deserialization of resizable objects to and from a file.
#include "corona-data-block.hpp"

// this did replace json-table, because the skip list doesn't work and its too damned slow anyway.
#include "corona-express-table.hpp"

// this is a contract of the express table,
// but implemented using SQL Server tables
#include "corona-sql.hpp"

// a json table here is a physical table in a file beginning at some point within it.
// it's ok, but it's actually quite slow and is not the simplest thing to thread.
#include "corona-json-table.hpp"

// a json object table here has a simpler key
#include "corona-json-object-table.hpp"

// this is a client for send grid
#include "corona-sendgrid-client.hpp"

// this is the database engine and it lets you create an object database 
// consisting of multiple classes
#include "corona-database-engine.hpp"

// the file monitor is a thing that lets you set up config files and watch for changes.
// the bus makes use of these to do the React style insta-edit
#include "corona-file-monitor.hpp"

// these are visual styles.  corona has a primitive style sheet system
// that is conceptually in between what HTML does - which makes a mess out of everything
// versus what XAML does, which is so simplistic.
// In Corona, a style sheet at its core basically implements the appearance of an element,
// specifying core border, background font and brushes to make a simple text box with 
// pretty things.
#include "corona-styles.hpp"

// In corona an application user interface is a presentation.
// there are pages with controls that can be displayed and navigated.
// so, in Windows, all the user events come from a win32 window via messages.
// presentation-events lets developers set up handlers for them and specifies
// the structure used to handle them.

#include "corona-presentation-events.hpp"

// this is a representation of a menu, that can be a win32 menu
#include "corona-presentation-menu.hpp"

// these are all the controls that can be on a presentation
#include "corona-presentation-controls-base.hpp"
#include "corona-comm-bus-commands.hpp"
#include "corona-presentation-controls-dx.hpp"
#include "corona-presentation-controls-dx-container.hpp"
#include "corona-presentation-controls-dx-text.hpp"
#include "corona-presentation-controls-win32.hpp"

// builder has some complex controls, but its main reason for existing
// is to separate control construction from the control tree itself.  
// if you are coding, it lets you use a fluid syntax
// if you just want to throw json at it and say build me a ui,
// it will do that too.
#include "corona-presentation-builder.hpp"

// a presentation page is a control hiearchy, referenced by a root
// and it can receive events from a win32 window.
// in mvc parlance, a presentation is a controller, and it is a controller
// in corona
#include "corona-presentation-page.hpp"
#include "corona-presentation.hpp"
#include "corona-directapplication.hpp"

// the easiest way to let an application be a web service.
// this little bad boy uses the Windows HTTP Server stack 
// and so yes a corona applications cheesy web server is 
// actually mission ready to go, and does all the right things
// well mostly, with threading and message handoff.
// https works.
#include "corona-httpserver.hpp"


// this is the http client to the corona service
#include "corona-corona-client.hpp"

// publicity stunts..  this implements the OLE drag and drop of us as a source target
#include "corona-publicity-stunts.hpp"

// the communication bus allows all the services in a corona 
// application to talk to each.  It also implements publish / subscribe.
// it's not really complete per se, but it is so much easier now
// that this will continue.
// this one is for GUI applications
#include "corona-comm-app-bus.hpp"

// the communication bus allows all the services in a corona 
// application to talk to each.  It also implements publish / subscribe.
// it's not really complete per se, but it is so much easier now
// that this will continue.
// this one is for Service applications 
#include "corona-comm-service-bus.hpp"

#endif
