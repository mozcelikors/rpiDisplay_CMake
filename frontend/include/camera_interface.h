#include <opencv2/core.hpp>
#include <semaphore.h>
#include <QGraphicsScene>

//#define CAMERA_DISPLAY_METHOD_1
#define CAMERA_DISPLAY_METHOD_2

typedef struct {
   int camera_active_f;   /* 1: Active 0: Non-active*/
   cv::Mat camera_image;
   sem_t    camera_sem;
   QGraphicsScene *camera_scene;
} CameraStreamInterface;

extern CameraStreamInterface csi;

