// source: http://dlib.net/dnn_semantic_segmentation_ex.h.html
// The contents of this file are in the public domain. See LICENSE_FOR_EXAMPLE_PROGRAMS.txt
/*
    Semantic segmentation using the PASCAL VOC2012 dataset.

    In segmentation, the task is to assign each pixel of an input image
    a label - for example, 'dog'.  Then, the idea is that neighboring
    pixels having the same label can be connected together to form a
    larger region, representing a complete (or partially occluded) dog.
    So technically, segmentation can be viewed as classification of
    individual pixels (using the relevant context in the input images),
    however the goal usually is to identify meaningful regions that
    represent complete entities of interest (such as dogs).

    Instructions how to run the example:
    1. Download the PASCAL VOC2012 data, and untar it somewhere.
       http://host.robots.ox.ac.uk/pascal/VOC/voc2012/VOCtrainval_11-May-2012.tar
    2. Build the dnn_semantic_segmentation_train_ex example program.
    3. Run:
       ./dnn_semantic_segmentation_train_ex /path/to/VOC2012
    4. Wait while the network is being trained.
    5. Build the dnn_semantic_segmentation_ex example program.
    6. Run:
       ./dnn_semantic_segmentation_ex /path/to/VOC2012-or-other-images

    An alternative to steps 2-4 above is to download a pre-trained network
    from here: http://dlib.net/files/semantic_segmentation_voc2012net.dnn

    It would be a good idea to become familiar with dlib's DNN tooling before reading this
    example.  So you should read dnn_introduction_ex.cpp and dnn_introduction2_ex.cpp
    before reading this example program.
*/

#ifndef DLIB_DNn_SEMANTIC_SEGMENTATION_EX_H_
#define DLIB_DNn_SEMANTIC_SEGMENTATION_EX_H_

#include <dlib/dnn.h>
#include <string>
#include <iostream>

// ----------------------------------------------------------------------------------------

inline bool operator == (const dlib::rgb_pixel& a, const dlib::rgb_pixel& b)
{
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}

// ----------------------------------------------------------------------------------------

// The PASCAL VOC2012 dataset contains 20 ground-truth classes + background.  Each class
// is represented using an RGB color value.  We associate each class also to an index in the
// range [0, 20], used internally by the network.

struct Voc2012class {
    Voc2012class(uint16_t index, const dlib::rgb_pixel& rgb_label, const std::string& classlabel)
        : index(index), rgb_label(rgb_label), classlabel(classlabel)
    {}

    // The index of the class. In the PASCAL VOC 2012 dataset, indexes from 0 to 20 are valid.
    const uint16_t index = 0;

    // The corresponding RGB representation of the class.
    const dlib::rgb_pixel rgb_label;

    // The label of the class in plain text.
    const std::string classlabel;
};

namespace {
constexpr int class_count = 256; // background + 20 classes

std::vector<Voc2012class> classes = {
    Voc2012class(0, dlib::rgb_pixel(0, 0, 0), ""), // background

    // The cream-colored `void' label is used in border regions and to mask difficult objects
    // (see http://host.robots.ox.ac.uk/pascal/VOC/voc2012/htmldoc/devkit_doc.html)
    Voc2012class(dlib::loss_multiclass_log_per_pixel_::label_to_ignore,
    dlib::rgb_pixel(224, 224, 192), "border"),

    Voc2012class(1,  dlib::rgb_pixel(128,   0,   0), "aeroplane"),
    Voc2012class(2,  dlib::rgb_pixel(  0, 128,   0), "bicycle"),
    Voc2012class(3,  dlib::rgb_pixel(128, 128,   0), "bird"),
    Voc2012class(4,  dlib::rgb_pixel(  0,   0, 128), "boat"),
    Voc2012class(5,  dlib::rgb_pixel(128,   0, 128), "bottle"),
    Voc2012class(6,  dlib::rgb_pixel(  0, 128, 128), "bus"),
    Voc2012class(7,  dlib::rgb_pixel(128, 128, 128), "car"),
    Voc2012class(8,  dlib::rgb_pixel( 64,   0,   0), "cat"),
    Voc2012class(9,  dlib::rgb_pixel(192,   0,   0), "chair"),
    Voc2012class(10, dlib::rgb_pixel( 64, 128,   0), "cow"),
    Voc2012class(11, dlib::rgb_pixel(192, 128,   0), "diningtable"),
    Voc2012class(12, dlib::rgb_pixel( 64,   0, 128), "dog"),
    Voc2012class(13, dlib::rgb_pixel(192,   0, 128), "horse"),
    Voc2012class(14, dlib::rgb_pixel( 64, 128, 128), "motorbike"),
    Voc2012class(15, dlib::rgb_pixel(192, 128, 128), "person"),
    Voc2012class(16, dlib::rgb_pixel(  0,  64,   0), "pottedplant"),
    Voc2012class(17, dlib::rgb_pixel(128,  64,   0), "sheep"),
    Voc2012class(18, dlib::rgb_pixel(  0, 192,   0), "sofa"),
    Voc2012class(19, dlib::rgb_pixel(128, 192,   0), "train"),
    Voc2012class(20, dlib::rgb_pixel(  0,  64, 128), "tvmonitor"),
    Voc2012class(21, dlib::rgb_pixel(  192,  192,  192), "class21"),
    Voc2012class(22, dlib::rgb_pixel(  192,  192,  192), "class22"),
    Voc2012class(23, dlib::rgb_pixel(  192,  192,  192), "class23"),
    Voc2012class(24, dlib::rgb_pixel(  192,  192,  192), "class24"),
    Voc2012class(25, dlib::rgb_pixel(  192,  192,  192), "class25"),
    Voc2012class(26, dlib::rgb_pixel(  192,  192,  192), "class26"),
    Voc2012class(27, dlib::rgb_pixel(  192,  192,  192), "class27"),
    Voc2012class(28, dlib::rgb_pixel(  192,  192,  192), "class28"),
    Voc2012class(29, dlib::rgb_pixel(  192,  192,  192), "class29"),
    Voc2012class(30, dlib::rgb_pixel(  192,  192,  192), "class30"),
    Voc2012class(31, dlib::rgb_pixel(  192,  192,  192), "class31"),
    Voc2012class(32, dlib::rgb_pixel(  192,  192,  192), "class32"),
    Voc2012class(33, dlib::rgb_pixel(  192,  192,  192), "class33"),
    Voc2012class(34, dlib::rgb_pixel(  192,  192,  192), "class34"),
    Voc2012class(35, dlib::rgb_pixel(  192,  192,  192), "class35"),
    Voc2012class(36, dlib::rgb_pixel(  192,  192,  192), "class36"),
    Voc2012class(37, dlib::rgb_pixel(  192,  192,  192), "class37"),
    Voc2012class(38, dlib::rgb_pixel(  192,  192,  192), "class38"),
    Voc2012class(39, dlib::rgb_pixel(  192,  192,  192), "class39"),
    Voc2012class(40, dlib::rgb_pixel(  192,  192,  192), "class40"),
    Voc2012class(41, dlib::rgb_pixel(  192,  192,  192), "class41"),
    Voc2012class(42, dlib::rgb_pixel(  192,  192,  192), "class42"),
    Voc2012class(43, dlib::rgb_pixel(  192,  192,  192), "class43"),
    Voc2012class(44, dlib::rgb_pixel(  192,  192,  192), "class44"),
    Voc2012class(45, dlib::rgb_pixel(  192,  192,  192), "class45"),
    Voc2012class(46, dlib::rgb_pixel(  192,  192,  192), "class46"),
    Voc2012class(47, dlib::rgb_pixel(  192,  192,  192), "class47"),
    Voc2012class(48, dlib::rgb_pixel(  192,  192,  192), "class48"),
    Voc2012class(49, dlib::rgb_pixel(  192,  192,  192), "class49"),
    Voc2012class(50, dlib::rgb_pixel(  192,  192,  192), "class50"),
    Voc2012class(51, dlib::rgb_pixel(  192,  192,  192), "class51"),
    Voc2012class(52, dlib::rgb_pixel(  192,  192,  192), "class52"),
    Voc2012class(53, dlib::rgb_pixel(  192,  192,  192), "class53"),
    Voc2012class(54, dlib::rgb_pixel(  192,  192,  192), "class54"),
    Voc2012class(55, dlib::rgb_pixel(  192,  192,  192), "class55"),
    Voc2012class(56, dlib::rgb_pixel(  192,  192,  192), "class56"),
    Voc2012class(57, dlib::rgb_pixel(  192,  192,  192), "class57"),
    Voc2012class(58, dlib::rgb_pixel(  192,  192,  192), "class58"),
    Voc2012class(59, dlib::rgb_pixel(  192,  192,  192), "class59"),
    Voc2012class(60, dlib::rgb_pixel(  192,  192,  192), "class60"),
    Voc2012class(61, dlib::rgb_pixel(  192,  192,  192), "class61"),
    Voc2012class(62, dlib::rgb_pixel(  192,  192,  192), "class62"),
    Voc2012class(63, dlib::rgb_pixel(  192,  192,  192), "class63"),
    Voc2012class(64, dlib::rgb_pixel(  192,  192,  192), "class64"),
    Voc2012class(65, dlib::rgb_pixel(  192,  192,  192), "class65"),
    Voc2012class(66, dlib::rgb_pixel(  192,  192,  192), "class66"),
    Voc2012class(67, dlib::rgb_pixel(  192,  192,  192), "class67"),
    Voc2012class(68, dlib::rgb_pixel(  192,  192,  192), "class68"),
    Voc2012class(69, dlib::rgb_pixel(  192,  192,  192), "class69"),
    Voc2012class(70, dlib::rgb_pixel(  192,  192,  192), "class70"),
    Voc2012class(71, dlib::rgb_pixel(  192,  192,  192), "class71"),
    Voc2012class(72, dlib::rgb_pixel(  192,  192,  192), "class72"),
    Voc2012class(73, dlib::rgb_pixel(  192,  192,  192), "class73"),
    Voc2012class(74, dlib::rgb_pixel(  192,  192,  192), "class74"),
    Voc2012class(75, dlib::rgb_pixel(  192,  192,  192), "class75"),
    Voc2012class(76, dlib::rgb_pixel(  192,  192,  192), "class76"),
    Voc2012class(77, dlib::rgb_pixel(  192,  192,  192), "class77"),
    Voc2012class(78, dlib::rgb_pixel(  192,  192,  192), "class78"),
    Voc2012class(79, dlib::rgb_pixel(  192,  192,  192), "class79"),
    Voc2012class(80, dlib::rgb_pixel(  192,  192,  192), "class80"),
    Voc2012class(81, dlib::rgb_pixel(  192,  192,  192), "class81"),
    Voc2012class(82, dlib::rgb_pixel(  192,  192,  192), "class82"),
    Voc2012class(83, dlib::rgb_pixel(  192,  192,  192), "class83"),
    Voc2012class(84, dlib::rgb_pixel(  192,  192,  192), "class84"),
    Voc2012class(85, dlib::rgb_pixel(  192,  192,  192), "class85"),
    Voc2012class(86, dlib::rgb_pixel(  192,  192,  192), "class86"),
    Voc2012class(87, dlib::rgb_pixel(  192,  192,  192), "class87"),
    Voc2012class(88, dlib::rgb_pixel(  192,  192,  192), "class88"),
    Voc2012class(89, dlib::rgb_pixel(  192,  192,  192), "class89"),
    Voc2012class(90, dlib::rgb_pixel(  192,  192,  192), "class90"),
    Voc2012class(91, dlib::rgb_pixel(  192,  192,  192), "class91"),
    Voc2012class(92, dlib::rgb_pixel(  192,  192,  192), "class92"),
    Voc2012class(93, dlib::rgb_pixel(  192,  192,  192), "class93"),
    Voc2012class(94, dlib::rgb_pixel(  192,  192,  192), "class94"),
    Voc2012class(95, dlib::rgb_pixel(  192,  192,  192), "class95"),
    Voc2012class(96, dlib::rgb_pixel(  192,  192,  192), "class96"),
    Voc2012class(97, dlib::rgb_pixel(  192,  192,  192), "class97"),
    Voc2012class(98, dlib::rgb_pixel(  192,  192,  192), "class98"),
    Voc2012class(99, dlib::rgb_pixel(  192,  192,  192), "class99"),
    Voc2012class(100, dlib::rgb_pixel(  192,  192,  192), "class100"),
    Voc2012class(101, dlib::rgb_pixel(  192,  192,  192), "class101"),
    Voc2012class(102, dlib::rgb_pixel(  192,  192,  192), "class102"),
    Voc2012class(103, dlib::rgb_pixel(  192,  192,  192), "class103"),
    Voc2012class(104, dlib::rgb_pixel(  192,  192,  192), "class104"),
    Voc2012class(105, dlib::rgb_pixel(  192,  192,  192), "class105"),
    Voc2012class(106, dlib::rgb_pixel(  192,  192,  192), "class106"),
    Voc2012class(107, dlib::rgb_pixel(  192,  192,  192), "class107"),
    Voc2012class(108, dlib::rgb_pixel(  192,  192,  192), "class108"),
    Voc2012class(109, dlib::rgb_pixel(  192,  192,  192), "class109"),
    Voc2012class(110, dlib::rgb_pixel(  192,  192,  192), "class110"),
    Voc2012class(111, dlib::rgb_pixel(  192,  192,  192), "class111"),
    Voc2012class(112, dlib::rgb_pixel(  192,  192,  192), "class112"),
    Voc2012class(113, dlib::rgb_pixel(  192,  192,  192), "class113"),
    Voc2012class(114, dlib::rgb_pixel(  192,  192,  192), "class114"),
    Voc2012class(115, dlib::rgb_pixel(  192,  192,  192), "class115"),
    Voc2012class(116, dlib::rgb_pixel(  192,  192,  192), "class116"),
    Voc2012class(117, dlib::rgb_pixel(  192,  192,  192), "class117"),
    Voc2012class(118, dlib::rgb_pixel(  192,  192,  192), "class118"),
    Voc2012class(119, dlib::rgb_pixel(  192,  192,  192), "class119"),
    Voc2012class(120, dlib::rgb_pixel(  192,  192,  192), "class120"),
    Voc2012class(121, dlib::rgb_pixel(  192,  192,  192), "class121"),
    Voc2012class(122, dlib::rgb_pixel(  192,  192,  192), "class122"),
    Voc2012class(123, dlib::rgb_pixel(  192,  192,  192), "class123"),
    Voc2012class(124, dlib::rgb_pixel(  192,  192,  192), "class124"),
    Voc2012class(125, dlib::rgb_pixel(  192,  192,  192), "class125"),
    Voc2012class(126, dlib::rgb_pixel(  192,  192,  192), "class126"),
    Voc2012class(127, dlib::rgb_pixel(  192,  192,  192), "class127"),
    Voc2012class(128, dlib::rgb_pixel(  192,  192,  192), "class128"),
    Voc2012class(129, dlib::rgb_pixel(  192,  192,  192), "class129"),
    Voc2012class(130, dlib::rgb_pixel(  192,  192,  192), "class130"),
    Voc2012class(131, dlib::rgb_pixel(  192,  192,  192), "class131"),
    Voc2012class(132, dlib::rgb_pixel(  192,  192,  192), "class132"),
    Voc2012class(133, dlib::rgb_pixel(  192,  192,  192), "class133"),
    Voc2012class(134, dlib::rgb_pixel(  192,  192,  192), "class134"),
    Voc2012class(135, dlib::rgb_pixel(  192,  192,  192), "class135"),
    Voc2012class(136, dlib::rgb_pixel(  192,  192,  192), "class136"),
    Voc2012class(137, dlib::rgb_pixel(  192,  192,  192), "class137"),
    Voc2012class(138, dlib::rgb_pixel(  192,  192,  192), "class138"),
    Voc2012class(139, dlib::rgb_pixel(  192,  192,  192), "class139"),
    Voc2012class(140, dlib::rgb_pixel(  192,  192,  192), "class140"),
    Voc2012class(141, dlib::rgb_pixel(  192,  192,  192), "class141"),
    Voc2012class(142, dlib::rgb_pixel(  192,  192,  192), "class142"),
    Voc2012class(143, dlib::rgb_pixel(  192,  192,  192), "class143"),
    Voc2012class(144, dlib::rgb_pixel(  192,  192,  192), "class144"),
    Voc2012class(145, dlib::rgb_pixel(  192,  192,  192), "class145"),
    Voc2012class(146, dlib::rgb_pixel(  192,  192,  192), "class146"),
    Voc2012class(147, dlib::rgb_pixel(  192,  192,  192), "class147"),
    Voc2012class(148, dlib::rgb_pixel(  192,  192,  192), "class148"),
    Voc2012class(149, dlib::rgb_pixel(  192,  192,  192), "class149"),
    Voc2012class(150, dlib::rgb_pixel(  192,  192,  192), "class150"),
    Voc2012class(151, dlib::rgb_pixel(  192,  192,  192), "class151"),
    Voc2012class(152, dlib::rgb_pixel(  192,  192,  192), "class152"),
    Voc2012class(153, dlib::rgb_pixel(  192,  192,  192), "class153"),
    Voc2012class(154, dlib::rgb_pixel(  192,  192,  192), "class154"),
    Voc2012class(155, dlib::rgb_pixel(  192,  192,  192), "class155"),
    Voc2012class(156, dlib::rgb_pixel(  192,  192,  192), "class156"),
    Voc2012class(157, dlib::rgb_pixel(  192,  192,  192), "class157"),
    Voc2012class(158, dlib::rgb_pixel(  192,  192,  192), "class158"),
    Voc2012class(159, dlib::rgb_pixel(  192,  192,  192), "class159"),
    Voc2012class(160, dlib::rgb_pixel(  192,  192,  192), "class160"),
    Voc2012class(161, dlib::rgb_pixel(  192,  192,  192), "class161"),
    Voc2012class(162, dlib::rgb_pixel(  192,  192,  192), "class162"),
    Voc2012class(163, dlib::rgb_pixel(  192,  192,  192), "class163"),
    Voc2012class(164, dlib::rgb_pixel(  192,  192,  192), "class164"),
    Voc2012class(165, dlib::rgb_pixel(  192,  192,  192), "class165"),
    Voc2012class(166, dlib::rgb_pixel(  192,  192,  192), "class166"),
    Voc2012class(167, dlib::rgb_pixel(  192,  192,  192), "class167"),
    Voc2012class(168, dlib::rgb_pixel(  192,  192,  192), "class168"),
    Voc2012class(169, dlib::rgb_pixel(  192,  192,  192), "class169"),
    Voc2012class(170, dlib::rgb_pixel(  192,  192,  192), "class170"),
    Voc2012class(171, dlib::rgb_pixel(  192,  192,  192), "class171"),
    Voc2012class(172, dlib::rgb_pixel(  192,  192,  192), "class172"),
    Voc2012class(173, dlib::rgb_pixel(  192,  192,  192), "class173"),
    Voc2012class(174, dlib::rgb_pixel(  192,  192,  192), "class174"),
    Voc2012class(175, dlib::rgb_pixel(  192,  192,  192), "class175"),
    Voc2012class(176, dlib::rgb_pixel(  192,  192,  192), "class176"),
    Voc2012class(177, dlib::rgb_pixel(  192,  192,  192), "class177"),
    Voc2012class(178, dlib::rgb_pixel(  192,  192,  192), "class178"),
    Voc2012class(179, dlib::rgb_pixel(  192,  192,  192), "class179"),
    Voc2012class(180, dlib::rgb_pixel(  192,  192,  192), "class180"),
    Voc2012class(181, dlib::rgb_pixel(  192,  192,  192), "class181"),
    Voc2012class(182, dlib::rgb_pixel(  192,  192,  192), "class182"),
    Voc2012class(183, dlib::rgb_pixel(  192,  192,  192), "class183"),
    Voc2012class(184, dlib::rgb_pixel(  192,  192,  192), "class184"),
    Voc2012class(185, dlib::rgb_pixel(  192,  192,  192), "class185"),
    Voc2012class(186, dlib::rgb_pixel(  192,  192,  192), "class186"),
    Voc2012class(187, dlib::rgb_pixel(  192,  192,  192), "class187"),
    Voc2012class(188, dlib::rgb_pixel(  192,  192,  192), "class188"),
    Voc2012class(189, dlib::rgb_pixel(  192,  192,  192), "class189"),
    Voc2012class(190, dlib::rgb_pixel(  192,  192,  192), "class190"),
    Voc2012class(191, dlib::rgb_pixel(  192,  192,  192), "class191"),
    Voc2012class(192, dlib::rgb_pixel(  192,  192,  192), "class192"),
    Voc2012class(193, dlib::rgb_pixel(  192,  192,  192), "class193"),
    Voc2012class(194, dlib::rgb_pixel(  192,  192,  192), "class194"),
    Voc2012class(195, dlib::rgb_pixel(  192,  192,  192), "class195"),
    Voc2012class(196, dlib::rgb_pixel(  192,  192,  192), "class196"),
    Voc2012class(197, dlib::rgb_pixel(  192,  192,  192), "class197"),
    Voc2012class(198, dlib::rgb_pixel(  192,  192,  192), "class198"),
    Voc2012class(199, dlib::rgb_pixel(  192,  192,  192), "class199"),
    Voc2012class(200, dlib::rgb_pixel(  192,  192,  192), "class200"),
    Voc2012class(201, dlib::rgb_pixel(  192,  192,  192), "class201"),
    Voc2012class(202, dlib::rgb_pixel(  192,  192,  192), "class202"),
    Voc2012class(203, dlib::rgb_pixel(  192,  192,  192), "class203"),
    Voc2012class(204, dlib::rgb_pixel(  192,  192,  192), "class204"),
    Voc2012class(205, dlib::rgb_pixel(  192,  192,  192), "class205"),
    Voc2012class(206, dlib::rgb_pixel(  192,  192,  192), "class206"),
    Voc2012class(207, dlib::rgb_pixel(  192,  192,  192), "class207"),
    Voc2012class(208, dlib::rgb_pixel(  192,  192,  192), "class208"),
    Voc2012class(209, dlib::rgb_pixel(  192,  192,  192), "class209"),
    Voc2012class(210, dlib::rgb_pixel(  192,  192,  192), "class210"),
    Voc2012class(211, dlib::rgb_pixel(  192,  192,  192), "class211"),
    Voc2012class(212, dlib::rgb_pixel(  192,  192,  192), "class212"),
    Voc2012class(213, dlib::rgb_pixel(  192,  192,  192), "class213"),
    Voc2012class(214, dlib::rgb_pixel(  192,  192,  192), "class214"),
    Voc2012class(215, dlib::rgb_pixel(  192,  192,  192), "class215"),
    Voc2012class(216, dlib::rgb_pixel(  192,  192,  192), "class216"),
    Voc2012class(217, dlib::rgb_pixel(  192,  192,  192), "class217"),
    Voc2012class(218, dlib::rgb_pixel(  192,  192,  192), "class218"),
    Voc2012class(219, dlib::rgb_pixel(  192,  192,  192), "class219"),
    Voc2012class(220, dlib::rgb_pixel(  192,  192,  192), "class220"),
    Voc2012class(221, dlib::rgb_pixel(  192,  192,  192), "class221"),
    Voc2012class(222, dlib::rgb_pixel(  192,  192,  192), "class222"),
    Voc2012class(223, dlib::rgb_pixel(  192,  192,  192), "class223"),
    Voc2012class(224, dlib::rgb_pixel(  192,  192,  192), "class224"),
    Voc2012class(225, dlib::rgb_pixel(  192,  192,  192), "class225"),
    Voc2012class(226, dlib::rgb_pixel(  192,  192,  192), "class226"),
    Voc2012class(227, dlib::rgb_pixel(  192,  192,  192), "class227"),
    Voc2012class(228, dlib::rgb_pixel(  192,  192,  192), "class228"),
    Voc2012class(229, dlib::rgb_pixel(  192,  192,  192), "class229"),
    Voc2012class(230, dlib::rgb_pixel(  192,  192,  192), "class230"),
    Voc2012class(231, dlib::rgb_pixel(  192,  192,  192), "class231"),
    Voc2012class(232, dlib::rgb_pixel(  192,  192,  192), "class232"),
    Voc2012class(233, dlib::rgb_pixel(  192,  192,  192), "class233"),
    Voc2012class(234, dlib::rgb_pixel(  192,  192,  192), "class234"),
    Voc2012class(235, dlib::rgb_pixel(  192,  192,  192), "class235"),
    Voc2012class(236, dlib::rgb_pixel(  192,  192,  192), "class236"),
    Voc2012class(237, dlib::rgb_pixel(  192,  192,  192), "class237"),
    Voc2012class(238, dlib::rgb_pixel(  192,  192,  192), "class238"),
    Voc2012class(239, dlib::rgb_pixel(  192,  192,  192), "class239"),
    Voc2012class(240, dlib::rgb_pixel(  192,  192,  192), "class240"),
    Voc2012class(241, dlib::rgb_pixel(  192,  192,  192), "class241"),
    Voc2012class(242, dlib::rgb_pixel(  192,  192,  192), "class242"),
    Voc2012class(243, dlib::rgb_pixel(  192,  192,  192), "class243"),
    Voc2012class(244, dlib::rgb_pixel(  192,  192,  192), "class244"),
    Voc2012class(245, dlib::rgb_pixel(  192,  192,  192), "class245"),
    Voc2012class(246, dlib::rgb_pixel(  192,  192,  192), "class246"),
    Voc2012class(247, dlib::rgb_pixel(  192,  192,  192), "class247"),
    Voc2012class(248, dlib::rgb_pixel(  192,  192,  192), "class248"),
    Voc2012class(249, dlib::rgb_pixel(  192,  192,  192), "class249"),
    Voc2012class(250, dlib::rgb_pixel(  192,  192,  192), "class250"),
    Voc2012class(251, dlib::rgb_pixel(  192,  192,  192), "class251"),
    Voc2012class(252, dlib::rgb_pixel(  192,  192,  192), "class252"),
    Voc2012class(253, dlib::rgb_pixel(  192,  192,  192), "class253"),
    Voc2012class(254, dlib::rgb_pixel(  192,  192,  192), "class254"),
    Voc2012class(255, dlib::rgb_pixel(  192,  192,  192), "class255"),
};

}

template <typename Predicate>
const Voc2012class& find_voc2012_class(Predicate predicate)
{
    const auto i = std::find_if(classes.begin(), classes.end(), predicate);

    if (i != classes.end())
    {
        return *i;
    }
    else
    {
        //return *i;
        throw std::runtime_error("Unable to find a matching VOC2012 class");
    }
}

// ----------------------------------------------------------------------------------------

// Introduce the building blocks used to define the segmentation network.
// The network first does residual downsampling (similar to the dnn_imagenet_(train_)ex 
// example program), and then residual upsampling. The network could be improved e.g.
// by introducing skip connections from the input image, and/or the first layers, to the
// last layer(s).  (See Long et al., Fully Convolutional Networks for Semantic Segmentation,
// https://people.eecs.berkeley.edu/~jonlong/long_shelhamer_fcn.pdf)

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using block = BN<dlib::con<N,3,3,1,1, dlib::relu<BN<dlib::con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET> 
using blockt = BN<dlib::cont<N,3,3,1,1,dlib::relu<BN<dlib::cont<N,3,3,stride,stride,SUBNET>>>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = dlib::add_prev1<block<N,BN,1,dlib::tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = dlib::add_prev2<dlib::avg_pool<2,2,2,2,dlib::skip1<dlib::tag2<block<N,BN,2,dlib::tag1<SUBNET>>>>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_up = dlib::add_prev2<dlib::cont<N,2,2,2,2,dlib::skip1<dlib::tag2<blockt<N,BN,2,dlib::tag1<SUBNET>>>>>>;

template <int N, typename SUBNET> using res       = dlib::relu<residual<block,N,dlib::bn_con,SUBNET>>;
template <int N, typename SUBNET> using ares      = dlib::relu<residual<block,N,dlib::affine,SUBNET>>;
template <int N, typename SUBNET> using res_down  = dlib::relu<residual_down<block,N,dlib::bn_con,SUBNET>>;
template <int N, typename SUBNET> using ares_down = dlib::relu<residual_down<block,N,dlib::affine,SUBNET>>;
template <int N, typename SUBNET> using res_up    = dlib::relu<residual_up<block,N,dlib::bn_con,SUBNET>>;
template <int N, typename SUBNET> using ares_up   = dlib::relu<residual_up<block,N,dlib::affine,SUBNET>>;

// ----------------------------------------------------------------------------------------

template <typename SUBNET> using res512 = res<512, SUBNET>;
template <typename SUBNET> using res256 = res<256, SUBNET>;
template <typename SUBNET> using res128 = res<128, SUBNET>;
template <typename SUBNET> using res64  = res<64, SUBNET>;
template <typename SUBNET> using ares512 = ares<512, SUBNET>;
template <typename SUBNET> using ares256 = ares<256, SUBNET>;
template <typename SUBNET> using ares128 = ares<128, SUBNET>;
template <typename SUBNET> using ares64  = ares<64, SUBNET>;


template <typename SUBNET> using level1 = dlib::repeat<2,res512,res_down<512,SUBNET>>;
template <typename SUBNET> using level2 = dlib::repeat<2,res256,res_down<256,SUBNET>>;
template <typename SUBNET> using level3 = dlib::repeat<2,res128,res_down<128,SUBNET>>;
template <typename SUBNET> using level4 = dlib::repeat<2,res64,res<64,SUBNET>>;

template <typename SUBNET> using alevel1 = dlib::repeat<2,ares512,ares_down<512,SUBNET>>;
template <typename SUBNET> using alevel2 = dlib::repeat<2,ares256,ares_down<256,SUBNET>>;
template <typename SUBNET> using alevel3 = dlib::repeat<2,ares128,ares_down<128,SUBNET>>;
template <typename SUBNET> using alevel4 = dlib::repeat<2,ares64,ares<64,SUBNET>>;

template <typename SUBNET> using level1t = dlib::repeat<2,res512,res_up<512,SUBNET>>;
template <typename SUBNET> using level2t = dlib::repeat<2,res256,res_up<256,SUBNET>>;
template <typename SUBNET> using level3t = dlib::repeat<2,res128,res_up<128,SUBNET>>;
template <typename SUBNET> using level4t = dlib::repeat<2,res64,res_up<64,SUBNET>>;

template <typename SUBNET> using alevel1t = dlib::repeat<2,ares512,ares_up<512,SUBNET>>;
template <typename SUBNET> using alevel2t = dlib::repeat<2,ares256,ares_up<256,SUBNET>>;
template <typename SUBNET> using alevel3t = dlib::repeat<2,ares128,ares_up<128,SUBNET>>;
template <typename SUBNET> using alevel4t = dlib::repeat<2,ares64,ares_up<64,SUBNET>>;

// ----------------------------------------------------------------------------------------

// training network type
using net_type = dlib::loss_multiclass_log_per_pixel<
dlib::cont<class_count,7,7,2,2,
level4t<level3t<level2t<level1t<
level1<level2<level3<level4<
dlib::max_pool<3,3,2,2,dlib::relu<dlib::bn_con<dlib::con<64,7,7,2,2,
dlib::input<dlib::matrix<dlib::rgb_pixel>>
>>>>>>>>>>>>>>;

// testing network type (replaced batch normalization with fixed affine transforms)
using anet_type = dlib::loss_multiclass_log_per_pixel<
dlib::cont<class_count,7,7,2,2,
alevel4t<alevel3t<alevel2t<alevel1t<
alevel1<alevel2<alevel3<alevel4<
dlib::max_pool<3,3,2,2,dlib::relu<dlib::affine<dlib::con<64,7,7,2,2,
dlib::input<dlib::matrix<dlib::rgb_pixel>>
>>>>>>>>>>>>>>;

// ----------------------------------------------------------------------------------------

#endif // DLIB_DNn_SEMANTIC_SEGMENTATION_EX_H_
