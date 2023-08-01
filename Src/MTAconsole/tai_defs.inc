#define default_mtsin_chan	"mtsin-chan:last<2h"
#define default_mtsin_mta	"mtsin-mta:last<2h"
#define default_mtsin_msg	"mtsin-msg:last<2h"

#define default_mtsout_chan	"mtsout-chan:age<2h,num<500,vol<1M,last<2h"
#define default_mtsout_mta	"mtsout-mta:age<2h,num<500,vol<1M,last<2h"
#define default_mtsout_msg	"mtsout-msg:age<2h,vol<1M"

#define default_mtsboth_chan	"mtsboth-chan:age<2h,vol<1M,num<500,last<2h"
#define default_mtsboth_mta	"mtsboth-mta:age<2h,vol<1M,num<500,last<2h"
#define default_mtsboth_msg	"mtsboth-msg:age<2h,vol<1M"

#define default_mtain_chan	"mtain-chan:last<2h"
#define default_mtain_mta	"mtain-mta:last<2h"
#define default_mtain_msg	"mtain-msg:last<2h"

#define default_mtaout_chan	"mtaout-chan:age<48h,vol<10M,num<1k,last<2h"
#define default_mtaout_mta	"mtaout-mta:age<48h,vol<10M,num<1k,last<2h"
#define default_mtaout_msg	"mtaout-msg:age<48h,vol<10M"

#define default_mtaboth_chan	"mtaboth-chan:age<48h,vol<10M,num<1k,last<2h"
#define default_mtaboth_mta	"mtaboth-mta:age<48h,vol<10M,num<1k,last<2h"
#define default_mtaboth_msg	"mtaboth-msg:age<48h,vol<10M"

#define default_internal_chan	"internal-chan:age<20,vol<1M,num<10,last<20"
#define default_internal_mta	"internal-mta:age<20,vol<1M,num<10,last<20"
#define default_internal_msg	"internal-msg:age<20,vol<1M"

#define default_passive_chan	"passive-chan:age<20,vol<1M,num<10,last<20"
#define default_passive_mta	"passive-mta:age<20,vol<1M,num<10,last<20"
#define default_passive_msg	"passive-msg:age<20,vol<1M"
