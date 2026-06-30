; ModuleID = 'mos-platform/atari7800/atari7800.c'
source_filename = "mos-platform/atari7800/atari7800.c"
target datalayout = "e-m:e-p:16:8-p1:8:8-i16:8-i32:8-i64:8-f32:8-f64:8-a:8-Fi8-n8"
target triple = "mos"

%struct.atari7800_maria_null_header = type { i8, i8 }
%struct.atari7800_maria_dll_entry = type { i8, i8, i8 }

@atari7800_scene_zones = internal global [28 x [96 x i8]] zeroinitializer, align 1
@atari7800_scene_null_zone = internal global %struct.atari7800_maria_null_header zeroinitializer, align 1
@atari7800_scene_display_list = internal global [33 x %struct.atari7800_maria_dll_entry] zeroinitializer, align 1
@atari7800_scene_active_zones_prev = internal unnamed_addr global i32 0, align 1
@atari7800_scene_active_zones_curr = internal unnamed_addr global i32 0, align 1
@atari7800_scene_zone_next_object = internal unnamed_addr global [28 x i8] zeroinitializer, align 1

; Function Attrs: nofree norecurse nounwind optsize memory(readwrite, target_mem: none)
define dso_local void @atari7800_init_system() local_unnamed_addr #0 {
  store volatile i8 7, ptr inttoptr (i16 1 to ptr), align 1, !tbaa !8
  store volatile i8 127, ptr inttoptr (i16 60 to ptr), align 4, !tbaa !8
  store volatile i8 0, ptr inttoptr (i16 56 to ptr), align 8, !tbaa !8
  store volatile i8 0, ptr inttoptr (i16 1 to ptr), align 1, !tbaa !8
  ret void
}

; Function Attrs: nofree norecurse nounwind optsize memory(readwrite, target_mem: none)
define dso_local void @atari7800_wait_vblank() local_unnamed_addr #0 {
  br label %1

1:                                                ; preds = %1, %0
  %2 = load volatile i8, ptr inttoptr (i16 40 to ptr), align 8, !tbaa !8
  %3 = icmp sgt i8 %2, -1
  br i1 %3, label %4, label %1, !llvm.loop !9

4:                                                ; preds = %1, %4
  %5 = load volatile i8, ptr inttoptr (i16 40 to ptr), align 8, !tbaa !8
  %6 = icmp sgt i8 %5, -1
  br i1 %6, label %4, label %7, !llvm.loop !11

7:                                                ; preds = %4
  ret void
}

; Function Attrs: nofree norecurse nounwind optsize memory(readwrite, target_mem: none)
define dso_local void @atari7800_configure_video(i16 noundef %0, i8 noundef zeroext %1, i8 noundef zeroext %2) local_unnamed_addr #0 {
  %4 = trunc i16 %0 to i8
  store volatile i8 %4, ptr inttoptr (i16 48 to ptr), align 16, !tbaa !8
  %5 = lshr i16 %0, 8
  %6 = trunc nuw i16 %5 to i8
  store volatile i8 %6, ptr inttoptr (i16 44 to ptr), align 4, !tbaa !8
  br label %7

7:                                                ; preds = %7, %3
  %8 = load volatile i8, ptr inttoptr (i16 40 to ptr), align 8, !tbaa !8
  %9 = icmp sgt i8 %8, -1
  br i1 %9, label %10, label %7, !llvm.loop !9

10:                                               ; preds = %7, %10
  %11 = load volatile i8, ptr inttoptr (i16 40 to ptr), align 8, !tbaa !8
  %12 = icmp sgt i8 %11, -1
  br i1 %12, label %10, label %13, !llvm.loop !11

13:                                               ; preds = %10
  store volatile i8 %1, ptr inttoptr (i16 60 to ptr), align 4, !tbaa !8
  store volatile i8 %2, ptr inttoptr (i16 32 to ptr), align 32, !tbaa !8
  ret void
}

; Function Attrs: nofree norecurse nounwind optsize memory(readwrite, target_mem: none)
define dso_local void @atari7800_init_160a(i16 noundef %0, i8 noundef zeroext %1) local_unnamed_addr #0 {
  store volatile i8 7, ptr inttoptr (i16 1 to ptr), align 1, !tbaa !8
  store volatile i8 127, ptr inttoptr (i16 60 to ptr), align 4, !tbaa !8
  store volatile i8 0, ptr inttoptr (i16 56 to ptr), align 8, !tbaa !8
  store volatile i8 0, ptr inttoptr (i16 1 to ptr), align 1, !tbaa !8
  %3 = trunc i16 %0 to i8
  store volatile i8 %3, ptr inttoptr (i16 48 to ptr), align 16, !tbaa !8
  %4 = lshr i16 %0, 8
  %5 = trunc nuw i16 %4 to i8
  store volatile i8 %5, ptr inttoptr (i16 44 to ptr), align 4, !tbaa !8
  br label %6

6:                                                ; preds = %6, %2
  %7 = load volatile i8, ptr inttoptr (i16 40 to ptr), align 8, !tbaa !8
  %8 = icmp sgt i8 %7, -1
  br i1 %8, label %9, label %6, !llvm.loop !9

9:                                                ; preds = %6, %9
  %10 = load volatile i8, ptr inttoptr (i16 40 to ptr), align 8, !tbaa !8
  %11 = icmp sgt i8 %10, -1
  br i1 %11, label %9, label %12, !llvm.loop !11

12:                                               ; preds = %9
  store volatile i8 64, ptr inttoptr (i16 60 to ptr), align 4, !tbaa !8
  store volatile i8 %1, ptr inttoptr (i16 32 to ptr), align 32, !tbaa !8
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind optsize memory(argmem: write)
define dso_local void @atari7800_maria_build_blank_ntsc(ptr noundef writeonly captures(none) initializes((0, 3)) %0, ptr noundef %1) local_unnamed_addr #1 {
  %3 = ptrtoint ptr %1 to i16
  store i8 8, ptr %0, align 1, !tbaa !12
  %4 = lshr i16 %3, 8
  %5 = trunc nuw i16 %4 to i8
  %6 = getelementptr inbounds nuw i8, ptr %0, i16 1
  store i8 %5, ptr %6, align 1, !tbaa !14
  %7 = trunc i16 %3 to i8
  %8 = getelementptr inbounds nuw i8, ptr %0, i16 2
  store i8 %7, ptr %8, align 1, !tbaa !15
  %9 = getelementptr nuw i8, ptr %0, i16 3
  %10 = getelementptr nuw i8, ptr %0, i16 4
  %11 = getelementptr nuw i8, ptr %0, i16 5
  br label %12

12:                                               ; preds = %2, %12
  %13 = phi i8 [ 0, %2 ], [ %21, %12 ]
  %14 = phi i8 [ 0, %2 ], [ %19, %12 ]
  %15 = zext nneg i8 %13 to i16
  %16 = getelementptr i8, ptr %9, i16 %15
  store i8 7, ptr %16, align 1, !tbaa !12
  %17 = getelementptr i8, ptr %10, i16 %15
  store i8 %5, ptr %17, align 1, !tbaa !14
  %18 = getelementptr i8, ptr %11, i16 %15
  store i8 %7, ptr %18, align 1, !tbaa !15
  %19 = add nuw nsw i8 %14, 1
  %20 = icmp eq i8 %19, 28
  %21 = add nuw nsw i8 %13, 3
  br i1 %20, label %22, label %12, !llvm.loop !16

22:                                               ; preds = %12
  %23 = getelementptr inbounds nuw i8, ptr %0, i16 87
  store i8 15, ptr %23, align 1, !tbaa !12
  %24 = getelementptr inbounds nuw i8, ptr %0, i16 88
  store i8 %5, ptr %24, align 1, !tbaa !14
  %25 = getelementptr inbounds nuw i8, ptr %0, i16 89
  store i8 %7, ptr %25, align 1, !tbaa !15
  %26 = getelementptr inbounds nuw i8, ptr %0, i16 90
  store i8 9, ptr %26, align 1, !tbaa !12
  %27 = getelementptr inbounds nuw i8, ptr %0, i16 91
  store i8 %5, ptr %27, align 1, !tbaa !14
  %28 = getelementptr inbounds nuw i8, ptr %0, i16 92
  store i8 %7, ptr %28, align 1, !tbaa !15
  %29 = getelementptr inbounds nuw i8, ptr %0, i16 93
  store i8 15, ptr %29, align 1, !tbaa !12
  %30 = getelementptr inbounds nuw i8, ptr %0, i16 94
  store i8 %5, ptr %30, align 1, !tbaa !14
  %31 = getelementptr inbounds nuw i8, ptr %0, i16 95
  store i8 %7, ptr %31, align 1, !tbaa !15
  %32 = getelementptr inbounds nuw i8, ptr %0, i16 96
  store i8 8, ptr %32, align 1, !tbaa !12
  %33 = getelementptr inbounds nuw i8, ptr %0, i16 97
  store i8 %5, ptr %33, align 1, !tbaa !14
  %34 = getelementptr inbounds nuw i8, ptr %0, i16 98
  store i8 %7, ptr %34, align 1, !tbaa !15
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind optsize memory(argmem: write)
define dso_local void @atari7800_maria_build_ntsc_single_zone(ptr noundef writeonly captures(none) initializes((0, 3)) %0, ptr noundef %1, i8 noundef zeroext %2, ptr noundef %3) local_unnamed_addr #1 {
  %5 = icmp ugt i8 %2, 27
  br i1 %5, label %6, label %7

6:                                                ; preds = %4
  br label %7

7:                                                ; preds = %6, %4
  %8 = phi i8 [ 0, %6 ], [ %2, %4 ]
  tail call void @atari7800_maria_build_blank_ntsc(ptr noundef %0, ptr noundef %1) #10
  %9 = zext nneg i8 %8 to i16
  %10 = getelementptr inbounds nuw [3 x i8], ptr %0, i16 %9
  %11 = getelementptr inbounds nuw i8, ptr %10, i16 3
  %12 = ptrtoint ptr %3 to i16
  store i8 7, ptr %11, align 1, !tbaa !12
  %13 = lshr i16 %12, 8
  %14 = trunc nuw i16 %13 to i8
  %15 = getelementptr inbounds nuw i8, ptr %10, i16 4
  store i8 %14, ptr %15, align 1, !tbaa !14
  %16 = trunc i16 %12 to i8
  %17 = getelementptr inbounds nuw i8, ptr %10, i16 5
  store i8 %16, ptr %17, align 1, !tbaa !15
  ret void
}

; Function Attrs: nofree norecurse nounwind optsize memory(readwrite, target_mem: none)
define dso_local void @atari7800_set_palette3(i8 noundef zeroext %0, i8 %1, i8 %2, i8 %3) local_unnamed_addr #0 {
  %5 = icmp ugt i8 %0, 7
  br i1 %5, label %13, label %6

6:                                                ; preds = %4
  %7 = shl nuw nsw i8 %0, 2
  %8 = or disjoint i8 %7, 33
  %9 = zext nneg i8 %8 to i16
  %10 = inttoptr i16 %9 to ptr
  store volatile i8 %1, ptr %10, align 1, !tbaa !8
  %11 = getelementptr inbounds nuw i8, ptr %10, i16 1
  store volatile i8 %2, ptr %11, align 2, !tbaa !8
  %12 = getelementptr inbounds nuw i8, ptr %10, i16 2
  store volatile i8 %3, ptr %12, align 1, !tbaa !8
  br label %13

13:                                               ; preds = %4, %6
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(argmem: write)
define dso_local void @atari7800_maria_build_sprite_zone5(ptr noundef writeonly captures(none) initializes((0, 7)) %0, i16 noundef %1, i8 noundef zeroext %2, i8 noundef zeroext %3, i8 noundef zeroext %4, i8 noundef zeroext %5) local_unnamed_addr #2 {
  %7 = trunc i16 %1 to i8
  store i8 %7, ptr %0, align 1, !tbaa !8
  %8 = getelementptr inbounds nuw i8, ptr %0, i16 1
  store i8 %2, ptr %8, align 1, !tbaa !8
  %9 = lshr i16 %1, 8
  %10 = trunc nuw i16 %9 to i8
  %11 = getelementptr inbounds nuw i8, ptr %0, i16 2
  store i8 %10, ptr %11, align 1, !tbaa !8
  %12 = shl i8 %3, 5
  %13 = and i8 %4, 31
  %14 = or disjoint i8 %13, %12
  %15 = getelementptr inbounds nuw i8, ptr %0, i16 3
  store i8 %14, ptr %15, align 1, !tbaa !8
  %16 = getelementptr inbounds nuw i8, ptr %0, i16 4
  store i8 %5, ptr %16, align 1, !tbaa !8
  %17 = getelementptr inbounds nuw i8, ptr %0, i16 5
  store i8 0, ptr %17, align 1, !tbaa !8
  %18 = getelementptr inbounds nuw i8, ptr %0, i16 6
  store i8 0, ptr %18, align 1, !tbaa !8
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(argmem: write)
define dso_local void @atari7800_maria_clear_zone(ptr noundef writeonly captures(none) %0, i16 noundef %1) local_unnamed_addr #2 {
  %3 = icmp eq i16 %1, 0
  br i1 %3, label %5, label %4

4:                                                ; preds = %2
  tail call void @llvm.memset.p0.i16(ptr align 1 %0, i8 0, i16 %1, i1 false), !tbaa !8
  br label %5

5:                                                ; preds = %4, %2
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(argmem: write)
define dso_local zeroext range(i8 0, 2) i8 @atari7800_maria_plot_sprite_zone5(ptr noundef writeonly captures(none) %0, i16 noundef %1, i8 noundef zeroext %2, i16 noundef %3, i8 noundef zeroext %4, i8 noundef zeroext %5, i8 noundef zeroext %6, i8 noundef zeroext %7) local_unnamed_addr #2 {
  %9 = zext i8 %2 to i16
  %10 = mul nuw nsw i16 %9, 5
  %11 = add nuw nsw i16 %10, 7
  %12 = icmp ugt i16 %11, %1
  br i1 %12, label %27, label %13

13:                                               ; preds = %8
  %14 = trunc i16 %3 to i8
  %15 = getelementptr inbounds nuw i8, ptr %0, i16 %10
  store i8 %14, ptr %15, align 1, !tbaa !8
  %16 = getelementptr inbounds nuw i8, ptr %15, i16 1
  store i8 %4, ptr %16, align 1, !tbaa !8
  %17 = lshr i16 %3, 8
  %18 = trunc nuw i16 %17 to i8
  %19 = getelementptr inbounds nuw i8, ptr %15, i16 2
  store i8 %18, ptr %19, align 1, !tbaa !8
  %20 = shl i8 %5, 5
  %21 = and i8 %6, 31
  %22 = or disjoint i8 %21, %20
  %23 = getelementptr inbounds nuw i8, ptr %15, i16 3
  store i8 %22, ptr %23, align 1, !tbaa !8
  %24 = getelementptr inbounds nuw i8, ptr %15, i16 4
  store i8 %7, ptr %24, align 1, !tbaa !8
  %25 = getelementptr inbounds nuw i8, ptr %15, i16 5
  store i8 0, ptr %25, align 1, !tbaa !8
  %26 = getelementptr inbounds nuw i8, ptr %15, i16 6
  store i8 0, ptr %26, align 1, !tbaa !8
  br label %27

27:                                               ; preds = %8, %13
  %28 = phi i8 [ 1, %13 ], [ 0, %8 ]
  ret i8 %28
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(argmem: readwrite)
define dso_local zeroext range(i8 0, 2) i8 @atari7800_maria_plot_sprite_asset_zone5(ptr noundef writeonly captures(none) %0, i16 noundef %1, i8 noundef zeroext %2, ptr noundef readonly captures(address_is_null) %3, i8 noundef zeroext %4) local_unnamed_addr #3 {
  %6 = icmp eq ptr %3, null
  br i1 %6, label %36, label %7

7:                                                ; preds = %5
  %8 = load ptr, ptr %3, align 1, !tbaa !17
  %9 = icmp eq ptr %8, null
  br i1 %9, label %36, label %10

10:                                               ; preds = %7
  %11 = zext i8 %2 to i16
  %12 = mul nuw nsw i16 %11, 5
  %13 = add nuw nsw i16 %12, 7
  %14 = icmp ugt i16 %13, %1
  br i1 %14, label %36, label %15

15:                                               ; preds = %10
  %16 = getelementptr inbounds nuw i8, ptr %3, i16 6
  %17 = load i8, ptr %16, align 1, !tbaa !21
  %18 = getelementptr inbounds nuw i8, ptr %3, i16 5
  %19 = load i8, ptr %18, align 1, !tbaa !22
  %20 = getelementptr inbounds nuw i8, ptr %3, i16 4
  %21 = load i8, ptr %20, align 1, !tbaa !23
  %22 = ptrtoint ptr %8 to i16
  %23 = trunc i16 %22 to i8
  %24 = getelementptr inbounds nuw i8, ptr %0, i16 %12
  store i8 %23, ptr %24, align 1, !tbaa !8
  %25 = getelementptr inbounds nuw i8, ptr %24, i16 1
  store i8 %21, ptr %25, align 1, !tbaa !8
  %26 = lshr i16 %22, 8
  %27 = trunc nuw i16 %26 to i8
  %28 = getelementptr inbounds nuw i8, ptr %24, i16 2
  store i8 %27, ptr %28, align 1, !tbaa !8
  %29 = shl i8 %19, 5
  %30 = and i8 %17, 31
  %31 = or disjoint i8 %29, %30
  %32 = getelementptr inbounds nuw i8, ptr %24, i16 3
  store i8 %31, ptr %32, align 1, !tbaa !8
  %33 = getelementptr inbounds nuw i8, ptr %24, i16 4
  store i8 %4, ptr %33, align 1, !tbaa !8
  %34 = getelementptr inbounds nuw i8, ptr %24, i16 5
  store i8 0, ptr %34, align 1, !tbaa !8
  %35 = getelementptr inbounds nuw i8, ptr %24, i16 6
  store i8 0, ptr %35, align 1, !tbaa !8
  br label %36

36:                                               ; preds = %15, %10, %5, %7
  %37 = phi i8 [ 0, %5 ], [ 0, %7 ], [ 1, %15 ], [ 0, %10 ]
  ret i8 %37
}

; Function Attrs: nofree norecurse nounwind optsize memory(readwrite, target_mem: none)
define dso_local void @atari7800_scene_init_160a(ptr noundef writeonly captures(none) initializes((0, 6)) %0, i8 noundef zeroext %1) local_unnamed_addr #0 {
  store ptr @atari7800_scene_zones, ptr %0, align 1, !tbaa !24
  %3 = getelementptr inbounds nuw i8, ptr %0, i16 2
  store i16 96, ptr %3, align 1, !tbaa !26
  %4 = getelementptr inbounds nuw i8, ptr %0, i16 4
  store i8 0, ptr %4, align 1, !tbaa !27
  %5 = getelementptr inbounds nuw i8, ptr %0, i16 5
  store i8 1, ptr %5, align 1, !tbaa !28
  store i8 0, ptr @atari7800_scene_null_zone, align 1, !tbaa !29
  store i8 0, ptr getelementptr inbounds nuw (i8, ptr @atari7800_scene_null_zone, i16 1), align 1, !tbaa !31
  tail call void @atari7800_maria_build_blank_ntsc(ptr noundef nonnull @atari7800_scene_display_list, ptr noundef nonnull @atari7800_scene_null_zone) #10
  store i32 0, ptr @atari7800_scene_active_zones_prev, align 1, !tbaa !32
  tail call void @llvm.memset.p0.i16(ptr noundef nonnull align 1 dereferenceable(2688) @atari7800_scene_zones, i8 0, i16 2688, i1 false), !tbaa !8
  store i32 0, ptr @atari7800_scene_active_zones_curr, align 1, !tbaa !32
  tail call void @llvm.memset.p0.i16(ptr noundef nonnull align 1 dereferenceable(28) @atari7800_scene_zone_next_object, i8 0, i16 28, i1 false), !tbaa !8
  store volatile i8 7, ptr inttoptr (i16 1 to ptr), align 1, !tbaa !8
  store volatile i8 127, ptr inttoptr (i16 60 to ptr), align 4, !tbaa !8
  store volatile i8 0, ptr inttoptr (i16 56 to ptr), align 8, !tbaa !8
  store volatile i8 0, ptr inttoptr (i16 1 to ptr), align 1, !tbaa !8
  store volatile i8 ptrtoint (ptr @atari7800_scene_display_list to i8), ptr inttoptr (i16 48 to ptr), align 16, !tbaa !8
  %6 = lshr i16 ptrtoint (ptr @atari7800_scene_display_list to i16), 8
  %7 = trunc nuw i16 %6 to i8
  store volatile i8 %7, ptr inttoptr (i16 44 to ptr), align 4, !tbaa !8
  br label %8

8:                                                ; preds = %8, %2
  %9 = load volatile i8, ptr inttoptr (i16 40 to ptr), align 8, !tbaa !8
  %10 = icmp sgt i8 %9, -1
  br i1 %10, label %11, label %8, !llvm.loop !9

11:                                               ; preds = %8, %11
  %12 = load volatile i8, ptr inttoptr (i16 40 to ptr), align 8, !tbaa !8
  %13 = icmp sgt i8 %12, -1
  br i1 %13, label %11, label %14, !llvm.loop !11

14:                                               ; preds = %11
  store volatile i8 64, ptr inttoptr (i16 60 to ptr), align 4, !tbaa !8
  store volatile i8 %1, ptr inttoptr (i16 32 to ptr), align 32, !tbaa !8
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(write, argmem: none, inaccessiblemem: none, target_mem: none)
define dso_local void @atari7800_scene_begin_frame(ptr noundef readnone captures(none) %0) local_unnamed_addr #4 {
  store i32 0, ptr @atari7800_scene_active_zones_curr, align 1, !tbaa !32
  tail call void @llvm.memset.p0.i16(ptr noundef nonnull align 1 dereferenceable(28) @atari7800_scene_zone_next_object, i8 0, i16 28, i1 false), !tbaa !8
  ret void
}

; Function Attrs: nofree norecurse nounwind optsize memory(readwrite, target_mem: none)
define dso_local void @atari7800_scene_set_palette(ptr noundef readnone captures(none) %0, i8 noundef zeroext %1, i8 %2, i8 %3, i8 %4) local_unnamed_addr #0 {
  %6 = icmp ugt i8 %1, 7
  br i1 %6, label %14, label %7

7:                                                ; preds = %5
  %8 = shl nuw nsw i8 %1, 2
  %9 = or disjoint i8 %8, 33
  %10 = zext nneg i8 %9 to i16
  %11 = inttoptr i16 %10 to ptr
  store volatile i8 %2, ptr %11, align 1, !tbaa !8
  %12 = getelementptr inbounds nuw i8, ptr %11, i16 1
  store volatile i8 %3, ptr %12, align 2, !tbaa !8
  %13 = getelementptr inbounds nuw i8, ptr %11, i16 2
  store volatile i8 %4, ptr %13, align 1, !tbaa !8
  br label %14

14:                                               ; preds = %5, %7
  ret void
}

; Function Attrs: nofree norecurse nosync nounwind optsize memory(readwrite, argmem: none, inaccessiblemem: none, target_mem: none)
define dso_local void @atari7800_scene_end_frame(ptr noundef readnone captures(none) %0) local_unnamed_addr #5 {
  %2 = load i32, ptr @atari7800_scene_active_zones_prev, align 1, !tbaa !32
  %3 = load i32, ptr @atari7800_scene_active_zones_curr, align 1
  %4 = lshr i16 ptrtoint (ptr @atari7800_scene_null_zone to i16), 8
  %5 = trunc nuw i16 %4 to i8
  br label %6

6:                                                ; preds = %1, %23
  %7 = phi i8 [ 0, %1 ], [ %26, %23 ]
  %8 = phi i8 [ 0, %1 ], [ %24, %23 ]
  %9 = zext nneg i8 %8 to i16
  %10 = zext nneg i8 %8 to i32
  %11 = shl nuw nsw i32 1, %10
  %12 = and i32 %2, %11
  %13 = icmp eq i32 %12, 0
  br i1 %13, label %23, label %14

14:                                               ; preds = %6
  %15 = and i32 %3, %11
  %16 = icmp eq i32 %15, 0
  br i1 %16, label %17, label %23

17:                                               ; preds = %14
  %18 = getelementptr inbounds nuw [96 x i8], ptr @atari7800_scene_zones, i16 %9
  tail call void @llvm.memset.p0.i16(ptr noundef nonnull align 1 dereferenceable(96) %18, i8 0, i16 96, i1 false), !tbaa !8
  %19 = zext nneg i8 %7 to i16
  %20 = getelementptr i8, ptr getelementptr inbounds nuw (i8, ptr @atari7800_scene_display_list, i16 3), i16 %19
  store i8 7, ptr %20, align 1, !tbaa !12
  %21 = getelementptr i8, ptr getelementptr inbounds nuw (i8, ptr @atari7800_scene_display_list, i16 4), i16 %19
  store i8 %5, ptr %21, align 1, !tbaa !14
  %22 = getelementptr i8, ptr getelementptr inbounds nuw (i8, ptr @atari7800_scene_display_list, i16 5), i16 %19
  store i8 ptrtoint (ptr @atari7800_scene_null_zone to i8), ptr %22, align 1, !tbaa !15
  br label %23

23:                                               ; preds = %17, %14, %6
  %24 = add nuw nsw i8 %8, 1
  %25 = icmp eq i8 %24, 28
  %26 = add nuw nsw i8 %7, 3
  br i1 %25, label %27, label %6, !llvm.loop !34

27:                                               ; preds = %23
  store i32 %3, ptr @atari7800_scene_active_zones_prev, align 1, !tbaa !32
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(argmem: write)
define dso_local void @atari7800_scene_begin(ptr noundef writeonly captures(none) initializes((0, 6)) %0, ptr noundef %1, i16 noundef %2) local_unnamed_addr #2 {
  store ptr %1, ptr %0, align 1, !tbaa !24
  %4 = getelementptr inbounds nuw i8, ptr %0, i16 2
  store i16 %2, ptr %4, align 1, !tbaa !26
  %5 = getelementptr inbounds nuw i8, ptr %0, i16 4
  store i8 0, ptr %5, align 1, !tbaa !27
  %6 = getelementptr inbounds nuw i8, ptr %0, i16 5
  store i8 0, ptr %6, align 1, !tbaa !28
  %7 = icmp eq i16 %2, 0
  br i1 %7, label %9, label %8

8:                                                ; preds = %3
  tail call void @llvm.memset.p0.i16(ptr align 1 %1, i8 0, i16 %2, i1 false), !tbaa !8
  br label %9

9:                                                ; preds = %3, %8
  ret void
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(write, argmem: readwrite, inaccessiblemem: none, target_mem: none)
define dso_local zeroext range(i8 0, 2) i8 @atari7800_scene_plotsprite(ptr noundef captures(none) %0, ptr noundef %1, i8 noundef zeroext %2, i8 noundef zeroext %3, i8 noundef zeroext %4, i8 noundef zeroext %5) local_unnamed_addr #6 {
  %7 = getelementptr inbounds nuw i8, ptr %0, i16 4
  %8 = load i8, ptr %7, align 1, !tbaa !27
  %9 = getelementptr inbounds nuw i8, ptr %0, i16 2
  %10 = load i16, ptr %9, align 1, !tbaa !26
  %11 = zext i8 %8 to i16
  %12 = mul nuw nsw i16 %11, 5
  %13 = add nuw nsw i16 %12, 7
  %14 = icmp ugt i16 %13, %10
  br i1 %14, label %33, label %15

15:                                               ; preds = %6
  %16 = load ptr, ptr %0, align 1, !tbaa !24
  %17 = ptrtoint ptr %1 to i16
  %18 = trunc i16 %17 to i8
  %19 = getelementptr inbounds nuw i8, ptr %16, i16 %12
  store i8 %18, ptr %19, align 1, !tbaa !8
  %20 = getelementptr inbounds nuw i8, ptr %19, i16 1
  store i8 %2, ptr %20, align 1, !tbaa !8
  %21 = lshr i16 %17, 8
  %22 = trunc nuw i16 %21 to i8
  %23 = getelementptr inbounds nuw i8, ptr %19, i16 2
  store i8 %22, ptr %23, align 1, !tbaa !8
  %24 = shl i8 %3, 5
  %25 = and i8 %4, 31
  %26 = or disjoint i8 %25, %24
  %27 = getelementptr inbounds nuw i8, ptr %19, i16 3
  store i8 %26, ptr %27, align 1, !tbaa !8
  %28 = getelementptr inbounds nuw i8, ptr %19, i16 4
  store i8 %5, ptr %28, align 1, !tbaa !8
  %29 = getelementptr inbounds nuw i8, ptr %19, i16 5
  store i8 0, ptr %29, align 1, !tbaa !8
  %30 = getelementptr inbounds nuw i8, ptr %19, i16 6
  store i8 0, ptr %30, align 1, !tbaa !8
  %31 = load i8, ptr %7, align 1, !tbaa !27
  %32 = add i8 %31, 1
  store i8 %32, ptr %7, align 1, !tbaa !27
  br label %33

33:                                               ; preds = %6, %15
  %34 = phi i8 [ 1, %15 ], [ 0, %6 ]
  ret i8 %34
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(readwrite, inaccessiblemem: none, target_mem: none)
define dso_local zeroext range(i8 0, 2) i8 @atari7800_scene_draw_sprite(ptr noundef captures(address_is_null) %0, ptr noundef readonly captures(address_is_null) %1, i8 noundef zeroext %2, i8 noundef zeroext %3) local_unnamed_addr #7 {
  %5 = icmp eq ptr %0, null
  br i1 %5, label %53, label %6

6:                                                ; preds = %4
  %7 = icmp eq ptr %1, null
  br i1 %7, label %53, label %8

8:                                                ; preds = %6
  %9 = getelementptr inbounds nuw i8, ptr %0, i16 5
  %10 = load i8, ptr %9, align 1, !tbaa !28
  %11 = icmp eq i8 %10, 0
  br i1 %11, label %12, label %23

12:                                               ; preds = %8
  %13 = getelementptr inbounds nuw i8, ptr %0, i16 4
  %14 = load i8, ptr %13, align 1, !tbaa !27
  %15 = load ptr, ptr %0, align 1, !tbaa !24
  %16 = getelementptr inbounds nuw i8, ptr %0, i16 2
  %17 = load i16, ptr %16, align 1, !tbaa !26
  %18 = tail call zeroext range(i8 0, 2) i8 @atari7800_maria_plot_sprite_asset_zone5(ptr noundef %15, i16 noundef %17, i8 noundef zeroext %14, ptr noundef nonnull readonly %1, i8 noundef zeroext %2) #10
  %19 = icmp eq i8 %18, 0
  br i1 %19, label %53, label %20

20:                                               ; preds = %12
  %21 = load i8, ptr %13, align 1, !tbaa !27
  %22 = add i8 %21, 1
  store i8 %22, ptr %13, align 1, !tbaa !27
  br label %53

23:                                               ; preds = %8
  %24 = lshr i8 %3, 3
  %25 = icmp ugt i8 %3, -33
  br i1 %25, label %26, label %27

26:                                               ; preds = %23
  br label %27

27:                                               ; preds = %26, %23
  %28 = phi i8 [ 27, %26 ], [ %24, %23 ]
  %29 = zext nneg i8 %28 to i16
  %30 = getelementptr inbounds nuw [96 x i8], ptr @atari7800_scene_zones, i16 %29
  %31 = getelementptr inbounds nuw i8, ptr @atari7800_scene_zone_next_object, i16 %29
  %32 = load i8, ptr %31, align 1, !tbaa !8
  %33 = tail call zeroext range(i8 0, 2) i8 @atari7800_maria_plot_sprite_asset_zone5(ptr noundef nonnull %30, i16 noundef 96, i8 noundef zeroext %32, ptr noundef nonnull readonly %1, i8 noundef zeroext %2) #10
  %34 = icmp eq i8 %33, 0
  br i1 %34, label %53, label %35

35:                                               ; preds = %27
  %36 = load i32, ptr @atari7800_scene_active_zones_curr, align 1, !tbaa !32
  %37 = zext nneg i8 %28 to i32
  %38 = shl nuw i32 1, %37
  %39 = and i32 %36, %38
  %40 = icmp eq i32 %39, 0
  br i1 %40, label %41, label %50

41:                                               ; preds = %35
  %42 = getelementptr inbounds nuw [3 x i8], ptr @atari7800_scene_display_list, i16 %29
  %43 = getelementptr inbounds nuw i8, ptr %42, i16 3
  %44 = ptrtoint ptr %30 to i16
  store i8 7, ptr %43, align 1, !tbaa !12
  %45 = lshr i16 %44, 8
  %46 = trunc nuw i16 %45 to i8
  %47 = getelementptr inbounds nuw i8, ptr %42, i16 4
  store i8 %46, ptr %47, align 1, !tbaa !14
  %48 = trunc i16 %44 to i8
  %49 = getelementptr inbounds nuw i8, ptr %42, i16 5
  store i8 %48, ptr %49, align 1, !tbaa !15
  br label %50

50:                                               ; preds = %41, %35
  %51 = add i8 %32, 1
  store i8 %51, ptr %31, align 1, !tbaa !8
  %52 = or i32 %36, %38
  store i32 %52, ptr @atari7800_scene_active_zones_curr, align 1, !tbaa !32
  br label %53

53:                                               ; preds = %27, %12, %4, %6, %50, %20
  %54 = phi i8 [ 0, %12 ], [ 1, %20 ], [ 0, %4 ], [ 1, %50 ], [ 0, %6 ], [ 0, %27 ]
  ret i8 %54
}

; Function Attrs: nofree norecurse nosync nounwind optsize memory(readwrite, inaccessiblemem: none, target_mem: none)
define dso_local zeroext range(i8 0, 2) i8 @atari7800_scene_draw_text(ptr noundef captures(address_is_null) %0, ptr noundef readonly captures(address_is_null) %1, i8 noundef zeroext %2, i8 noundef zeroext %3, ptr noundef readonly captures(address_is_null) %4) local_unnamed_addr #8 {
  %6 = icmp eq ptr %0, null
  br i1 %6, label %184, label %7

7:                                                ; preds = %5
  %8 = icmp eq ptr %1, null
  br i1 %8, label %184, label %9

9:                                                ; preds = %7
  %10 = icmp eq ptr %4, null
  br i1 %10, label %184, label %11

11:                                               ; preds = %9
  %12 = load ptr, ptr %1, align 1, !tbaa !35
  %13 = icmp eq ptr %12, null
  br i1 %13, label %184, label %14

14:                                               ; preds = %11
  %15 = getelementptr inbounds nuw i8, ptr %1, i16 6
  %16 = load i8, ptr %15, align 1, !tbaa !37
  %17 = icmp eq i8 %16, 0
  br i1 %17, label %184, label %18

18:                                               ; preds = %14
  %19 = getelementptr inbounds nuw i8, ptr %1, i16 12
  %20 = load i8, ptr %19, align 1, !tbaa !38
  %21 = icmp eq i8 %20, 0
  br i1 %21, label %184, label %22

22:                                               ; preds = %18
  %23 = getelementptr inbounds nuw i8, ptr %0, i16 4
  %24 = load i32, ptr @atari7800_scene_active_zones_curr, align 1
  %25 = getelementptr inbounds nuw i8, ptr %1, i16 13
  %26 = getelementptr inbounds nuw i8, ptr %1, i16 14
  %27 = getelementptr inbounds nuw i8, ptr %1, i16 2
  %28 = getelementptr inbounds nuw i8, ptr %1, i16 4
  %29 = getelementptr inbounds nuw i8, ptr %1, i16 5
  %30 = getelementptr inbounds nuw i8, ptr %0, i16 5
  %31 = getelementptr inbounds nuw i8, ptr %1, i16 9
  %32 = getelementptr inbounds nuw i8, ptr %1, i16 10
  %33 = getelementptr inbounds nuw i8, ptr %1, i16 11
  %34 = getelementptr inbounds nuw i8, ptr %0, i16 2
  br label %35

35:                                               ; preds = %22, %174
  %36 = phi i8 [ %175, %174 ], [ %20, %22 ]
  %37 = phi i32 [ %176, %174 ], [ %24, %22 ]
  %38 = phi i8 [ %177, %174 ], [ -1, %22 ]
  %39 = phi ptr [ %178, %174 ], [ null, %22 ]
  %40 = phi i8 [ %179, %174 ], [ 0, %22 ]
  %41 = phi i16 [ %180, %174 ], [ 0, %22 ]
  %42 = phi i8 [ %181, %174 ], [ %3, %22 ]
  %43 = phi i8 [ %182, %174 ], [ %2, %22 ]
  %44 = phi ptr [ %183, %174 ], [ %4, %22 ]
  %45 = load i8, ptr %44, align 1, !tbaa !8
  %46 = zext i8 %45 to i16
  switch i8 %45, label %57 [
    i8 0, label %184
    i8 10, label %47
    i8 32, label %50
  ]

47:                                               ; preds = %35
  %48 = load i8, ptr %26, align 1, !tbaa !39
  %49 = add i8 %48, %42
  br label %174, !llvm.loop !40

50:                                               ; preds = %35
  %51 = load i8, ptr %25, align 1, !tbaa !41
  %52 = icmp eq i8 %51, 0
  br i1 %52, label %53, label %54

53:                                               ; preds = %50
  br label %54

54:                                               ; preds = %50, %53
  %55 = phi i8 [ %36, %53 ], [ %51, %50 ]
  %56 = add i8 %55, %43
  br label %174

57:                                               ; preds = %35
  %58 = load ptr, ptr %27, align 1, !tbaa !42
  %59 = icmp eq ptr %58, null
  br i1 %59, label %66, label %60

60:                                               ; preds = %57
  %61 = load i8, ptr %28, align 1, !tbaa !43
  %62 = icmp ult i8 %45, %61
  br i1 %62, label %63, label %66

63:                                               ; preds = %60
  %64 = getelementptr inbounds nuw i8, ptr %58, i16 %46
  %65 = load i8, ptr %64, align 1, !tbaa !8
  br label %71

66:                                               ; preds = %60, %57
  %67 = load i8, ptr %29, align 1, !tbaa !44
  %68 = icmp ult i8 %45, %67
  br i1 %68, label %166, label %69

69:                                               ; preds = %66
  %70 = sub nuw i8 %45, %67
  br label %71

71:                                               ; preds = %69, %63
  %72 = phi i8 [ %65, %63 ], [ %70, %69 ]
  %73 = load i8, ptr %15, align 1, !tbaa !37
  %74 = icmp ult i8 %72, %73
  br i1 %74, label %75, label %166

75:                                               ; preds = %71
  %76 = zext i8 %72 to i16
  %77 = load ptr, ptr %1, align 1, !tbaa !35
  %78 = ptrtoint ptr %77 to i16
  %79 = shl nuw nsw i16 %76, 1
  %80 = add i16 %79, %78
  %81 = load i8, ptr %30, align 1, !tbaa !28
  %82 = icmp eq i8 %81, 0
  br i1 %82, label %83, label %110

83:                                               ; preds = %75
  %84 = load i8, ptr %23, align 1, !tbaa !27
  %85 = load i16, ptr %34, align 1, !tbaa !26
  %86 = zext i8 %84 to i16
  %87 = mul nuw nsw i16 %86, 5
  %88 = add nuw nsw i16 %87, 7
  %89 = icmp ugt i16 %88, %85
  br i1 %89, label %184, label %90

90:                                               ; preds = %83
  %91 = load i8, ptr %33, align 1, !tbaa !45
  %92 = load i8, ptr %32, align 1, !tbaa !46
  %93 = load i8, ptr %31, align 1, !tbaa !47
  %94 = load ptr, ptr %0, align 1, !tbaa !24
  %95 = trunc i16 %80 to i8
  %96 = getelementptr inbounds nuw i8, ptr %94, i16 %87
  store i8 %95, ptr %96, align 1, !tbaa !8
  %97 = getelementptr inbounds nuw i8, ptr %96, i16 1
  store i8 %93, ptr %97, align 1, !tbaa !8
  %98 = lshr i16 %80, 8
  %99 = trunc nuw i16 %98 to i8
  %100 = getelementptr inbounds nuw i8, ptr %96, i16 2
  store i8 %99, ptr %100, align 1, !tbaa !8
  %101 = shl i8 %92, 5
  %102 = and i8 %91, 31
  %103 = or disjoint i8 %101, %102
  %104 = getelementptr inbounds nuw i8, ptr %96, i16 3
  store i8 %103, ptr %104, align 1, !tbaa !8
  %105 = getelementptr inbounds nuw i8, ptr %96, i16 4
  store i8 %43, ptr %105, align 1, !tbaa !8
  %106 = getelementptr inbounds nuw i8, ptr %96, i16 5
  store i8 0, ptr %106, align 1, !tbaa !8
  %107 = getelementptr inbounds nuw i8, ptr %96, i16 6
  store i8 0, ptr %107, align 1, !tbaa !8
  %108 = load i8, ptr %23, align 1, !tbaa !27
  %109 = add i8 %108, 1
  store i8 %109, ptr %23, align 1, !tbaa !27
  br label %166

110:                                              ; preds = %75
  %111 = lshr i8 %42, 3
  %112 = icmp ugt i8 %42, -33
  br i1 %112, label %113, label %114

113:                                              ; preds = %110
  br label %114

114:                                              ; preds = %113, %110
  %115 = phi i8 [ 27, %113 ], [ %111, %110 ]
  %116 = zext nneg i8 %115 to i16
  %117 = icmp eq i8 %115, %38
  br i1 %117, label %124, label %118

118:                                              ; preds = %114
  %119 = getelementptr inbounds nuw [96 x i8], ptr @atari7800_scene_zones, i16 %116
  %120 = getelementptr inbounds nuw i8, ptr @atari7800_scene_zone_next_object, i16 %116
  %121 = load i8, ptr %120, align 1, !tbaa !8
  %122 = zext i8 %121 to i16
  %123 = mul nuw nsw i16 %122, 5
  br label %124

124:                                              ; preds = %118, %114
  %125 = phi i8 [ %115, %118 ], [ %38, %114 ]
  %126 = phi ptr [ %119, %118 ], [ %39, %114 ]
  %127 = phi i8 [ %121, %118 ], [ %40, %114 ]
  %128 = phi i16 [ %123, %118 ], [ %41, %114 ]
  %129 = add i16 %128, 7
  %130 = icmp ult i16 %129, 97
  br i1 %130, label %131, label %184

131:                                              ; preds = %124
  %132 = trunc i16 %80 to i8
  %133 = getelementptr inbounds nuw i8, ptr %126, i16 %128
  store i8 %132, ptr %133, align 1, !tbaa !8
  %134 = load i8, ptr %31, align 1, !tbaa !47
  %135 = getelementptr i8, ptr %133, i16 1
  store i8 %134, ptr %135, align 1, !tbaa !8
  %136 = lshr i16 %80, 8
  %137 = trunc nuw i16 %136 to i8
  %138 = getelementptr i8, ptr %133, i16 2
  store i8 %137, ptr %138, align 1, !tbaa !8
  %139 = load i8, ptr %32, align 1, !tbaa !46
  %140 = load i8, ptr %33, align 1, !tbaa !45
  %141 = shl i8 %139, 5
  %142 = and i8 %140, 31
  %143 = or disjoint i8 %142, %141
  %144 = getelementptr i8, ptr %133, i16 3
  store i8 %143, ptr %144, align 1, !tbaa !8
  %145 = getelementptr i8, ptr %133, i16 4
  store i8 %43, ptr %145, align 1, !tbaa !8
  %146 = add nsw i16 %128, 5
  %147 = getelementptr inbounds nuw i8, ptr %126, i16 %146
  store i8 0, ptr %147, align 1, !tbaa !8
  %148 = getelementptr i8, ptr %133, i16 6
  store i8 0, ptr %148, align 1, !tbaa !8
  %149 = zext nneg i8 %115 to i32
  %150 = shl nuw i32 1, %149
  %151 = and i32 %37, %150
  %152 = icmp eq i32 %151, 0
  br i1 %152, label %153, label %162

153:                                              ; preds = %131
  %154 = getelementptr inbounds nuw [3 x i8], ptr @atari7800_scene_display_list, i16 %116
  %155 = getelementptr inbounds nuw i8, ptr %154, i16 3
  %156 = ptrtoint ptr %126 to i16
  store i8 7, ptr %155, align 1, !tbaa !12
  %157 = lshr i16 %156, 8
  %158 = trunc nuw i16 %157 to i8
  %159 = getelementptr inbounds nuw i8, ptr %154, i16 4
  store i8 %158, ptr %159, align 1, !tbaa !14
  %160 = trunc i16 %156 to i8
  %161 = getelementptr inbounds nuw i8, ptr %154, i16 5
  store i8 %160, ptr %161, align 1, !tbaa !15
  br label %162

162:                                              ; preds = %131, %153
  %163 = add i8 %127, 1
  %164 = getelementptr inbounds nuw i8, ptr @atari7800_scene_zone_next_object, i16 %116
  store i8 %163, ptr %164, align 1, !tbaa !8
  %165 = or i32 %37, %150
  store i32 %165, ptr @atari7800_scene_active_zones_curr, align 1, !tbaa !32
  br label %166

166:                                              ; preds = %66, %71, %162, %90
  %167 = phi i32 [ %165, %162 ], [ %37, %90 ], [ %37, %71 ], [ %37, %66 ]
  %168 = phi i8 [ %125, %162 ], [ %38, %90 ], [ %38, %71 ], [ %38, %66 ]
  %169 = phi ptr [ %126, %162 ], [ %39, %90 ], [ %39, %71 ], [ %39, %66 ]
  %170 = phi i8 [ %163, %162 ], [ %40, %90 ], [ %40, %71 ], [ %40, %66 ]
  %171 = phi i16 [ %146, %162 ], [ %41, %90 ], [ %41, %71 ], [ %41, %66 ]
  %172 = load i8, ptr %19, align 1, !tbaa !38
  %173 = add i8 %172, %43
  br label %174

174:                                              ; preds = %166, %54, %47
  %175 = phi i8 [ %36, %47 ], [ %36, %54 ], [ %172, %166 ]
  %176 = phi i32 [ %37, %47 ], [ %37, %54 ], [ %167, %166 ]
  %177 = phi i8 [ -1, %47 ], [ %38, %54 ], [ %168, %166 ]
  %178 = phi ptr [ %39, %47 ], [ %39, %54 ], [ %169, %166 ]
  %179 = phi i8 [ %40, %47 ], [ %40, %54 ], [ %170, %166 ]
  %180 = phi i16 [ %41, %47 ], [ %41, %54 ], [ %171, %166 ]
  %181 = phi i8 [ %49, %47 ], [ %42, %54 ], [ %42, %166 ]
  %182 = phi i8 [ %2, %47 ], [ %56, %54 ], [ %173, %166 ]
  %183 = getelementptr inbounds nuw i8, ptr %44, i16 1
  br label %35

184:                                              ; preds = %83, %124, %35, %5, %7, %9, %11, %14, %18
  %185 = phi i8 [ 0, %5 ], [ 0, %18 ], [ 0, %14 ], [ 0, %11 ], [ 0, %9 ], [ 0, %7 ], [ 0, %83 ], [ 0, %124 ], [ 1, %35 ]
  ret i8 %185
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind optsize willreturn memory(write, argmem: readwrite, inaccessiblemem: none, target_mem: none)
define dso_local zeroext range(i8 0, 2) i8 @atari7800_scene_plotchars(ptr noundef captures(none) %0, ptr noundef %1, i8 noundef zeroext %2, i8 noundef zeroext %3, i8 noundef zeroext %4, i8 noundef zeroext %5) local_unnamed_addr #6 {
  %7 = getelementptr inbounds nuw i8, ptr %0, i16 4
  %8 = load i8, ptr %7, align 1, !tbaa !27
  %9 = getelementptr inbounds nuw i8, ptr %0, i16 2
  %10 = load i16, ptr %9, align 1, !tbaa !26
  %11 = zext i8 %8 to i16
  %12 = mul nuw nsw i16 %11, 5
  %13 = add nuw nsw i16 %12, 7
  %14 = icmp ugt i16 %13, %10
  br i1 %14, label %33, label %15

15:                                               ; preds = %6
  %16 = load ptr, ptr %0, align 1, !tbaa !24
  %17 = ptrtoint ptr %1 to i16
  %18 = trunc i16 %17 to i8
  %19 = getelementptr inbounds nuw i8, ptr %16, i16 %12
  store i8 %18, ptr %19, align 1, !tbaa !8
  %20 = getelementptr inbounds nuw i8, ptr %19, i16 1
  store i8 %2, ptr %20, align 1, !tbaa !8
  %21 = lshr i16 %17, 8
  %22 = trunc nuw i16 %21 to i8
  %23 = getelementptr inbounds nuw i8, ptr %19, i16 2
  store i8 %22, ptr %23, align 1, !tbaa !8
  %24 = shl i8 %3, 5
  %25 = and i8 %4, 31
  %26 = or disjoint i8 %25, %24
  %27 = getelementptr inbounds nuw i8, ptr %19, i16 3
  store i8 %26, ptr %27, align 1, !tbaa !8
  %28 = getelementptr inbounds nuw i8, ptr %19, i16 4
  store i8 %5, ptr %28, align 1, !tbaa !8
  %29 = getelementptr inbounds nuw i8, ptr %19, i16 5
  store i8 0, ptr %29, align 1, !tbaa !8
  %30 = getelementptr inbounds nuw i8, ptr %19, i16 6
  store i8 0, ptr %30, align 1, !tbaa !8
  %31 = load i8, ptr %7, align 1, !tbaa !27
  %32 = add i8 %31, 1
  store i8 %32, ptr %7, align 1, !tbaa !27
  br label %33

33:                                               ; preds = %6, %15
  %34 = phi i8 [ 1, %15 ], [ 0, %6 ]
  ret i8 %34
}

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: write)
declare void @llvm.memset.p0.i16(ptr writeonly captures(none), i8, i16, i1 immarg) #9

attributes #0 = { nofree norecurse nounwind optsize memory(readwrite, target_mem: none) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #1 = { nofree norecurse nosync nounwind optsize memory(argmem: write) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #2 = { mustprogress nofree norecurse nosync nounwind optsize willreturn memory(argmem: write) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { mustprogress nofree norecurse nosync nounwind optsize willreturn memory(argmem: readwrite) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #4 = { mustprogress nofree norecurse nosync nounwind optsize willreturn memory(write, argmem: none, inaccessiblemem: none, target_mem: none) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #5 = { nofree norecurse nosync nounwind optsize memory(readwrite, argmem: none, inaccessiblemem: none, target_mem: none) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #6 = { mustprogress nofree norecurse nosync nounwind optsize willreturn memory(write, argmem: readwrite, inaccessiblemem: none, target_mem: none) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #7 = { mustprogress nofree norecurse nosync nounwind optsize willreturn memory(readwrite, inaccessiblemem: none, target_mem: none) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #8 = { nofree norecurse nosync nounwind optsize memory(readwrite, inaccessiblemem: none, target_mem: none) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #9 = { nocallback nofree nounwind willreturn memory(argmem: write) }
attributes #10 = { optsize }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}
!llvm.errno.tbaa = !{!4}

!0 = !{i32 7, !"frame-pointer", i32 2}
!1 = !{i32 1, !"ThinLTO", i32 0}
!2 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!3 = !{!"clang version 23.0.0git (https://github.com/llvm-mos/llvm-mos c798c31416f72b395c658b5502d281a162387ab1)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!6, !6, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = !{!13, !6, i64 0}
!13 = !{!"atari7800_maria_dll_entry", !6, i64 0, !6, i64 1, !6, i64 2}
!14 = !{!13, !6, i64 1}
!15 = !{!13, !6, i64 2}
!16 = distinct !{!16, !10}
!17 = !{!18, !19, i64 0}
!18 = !{!"atari7800_sprite_asset", !19, i64 0, !6, i64 2, !6, i64 3, !6, i64 4, !6, i64 5, !6, i64 6, !6, i64 7}
!19 = !{!"p1 omnipotent char", !20, i64 0}
!20 = !{!"any pointer", !6, i64 0}
!21 = !{!18, !6, i64 6}
!22 = !{!18, !6, i64 5}
!23 = !{!18, !6, i64 4}
!24 = !{!25, !19, i64 0}
!25 = !{!"atari7800_scene", !19, i64 0, !5, i64 2, !6, i64 4, !6, i64 5}
!26 = !{!25, !5, i64 2}
!27 = !{!25, !6, i64 4}
!28 = !{!25, !6, i64 5}
!29 = !{!30, !6, i64 0}
!30 = !{!"atari7800_maria_null_header", !6, i64 0, !6, i64 1}
!31 = !{!30, !6, i64 1}
!32 = !{!33, !33, i64 0}
!33 = !{!"long", !6, i64 0}
!34 = distinct !{!34, !10}
!35 = !{!36, !19, i64 0}
!36 = !{!"atari7800_font_descriptor", !19, i64 0, !19, i64 2, !6, i64 4, !6, i64 5, !6, i64 6, !6, i64 7, !6, i64 8, !6, i64 9, !6, i64 10, !6, i64 11, !6, i64 12, !6, i64 13, !6, i64 14}
!37 = !{!36, !6, i64 6}
!38 = !{!36, !6, i64 12}
!39 = !{!36, !6, i64 14}
!40 = distinct !{!40, !10}
!41 = !{!36, !6, i64 13}
!42 = !{!36, !19, i64 2}
!43 = !{!36, !6, i64 4}
!44 = !{!36, !6, i64 5}
!45 = !{!36, !6, i64 11}
!46 = !{!36, !6, i64 10}
!47 = !{!36, !6, i64 9}

^0 = module: (path: "[Regular LTO]", hash: (0, 0, 0, 0, 0))
^1 = gv: (name: "atari7800_scene_zone_next_object", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 23962324553747553
^2 = gv: (name: "atari7800_scene_draw_sprite", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 57, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), calls: ((callee: ^6, tail: 1)), refs: (^16, ^1, ^14, ^21)))) ; guid = 494824695760564597
^3 = gv: (name: "atari7800_init_system", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 5, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 582917906497579109
^4 = gv: (name: "atari7800_wait_vblank", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 8, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 1069126409930794999
^5 = gv: (name: "atari7800_maria_clear_zone", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 5, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 1976945022743374768
^6 = gv: (name: "atari7800_maria_plot_sprite_asset_zone5", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 40, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 2069580684517917474
^7 = gv: (name: "atari7800_maria_build_blank_ntsc", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 51, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 2351686643285016436
^8 = gv: (name: "atari7800_scene_plotsprite", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 37, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 2464395537847532134
^9 = gv: (name: "atari7800_set_palette3", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 13, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 3310780590327362190
^10 = gv: (name: "atari7800_scene_init_160a", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 32, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), calls: ((callee: ^7, tail: 1)), refs: (^16, ^17, ^21, ^12, ^14, ^1)))) ; guid = 3805461889973060826
^11 = gv: (name: "atari7800_scene_begin_frame", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 3, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), refs: (^14, ^1)))) ; guid = 3915946421198288063
^12 = gv: (name: "atari7800_scene_active_zones_prev", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 3973908694907407446
^13 = gv: (name: "atari7800_scene_begin", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 12, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 5262556453624492715
^14 = gv: (name: "atari7800_scene_active_zones_curr", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 5821592117430679290
^15 = gv: (name: "atari7800_maria_build_sprite_zone5", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 20, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 6922751631088812303
^16 = gv: (name: "atari7800_scene_zones", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 7290661545248435150
^17 = gv: (name: "atari7800_scene_null_zone", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 9469744455926839642
^18 = gv: (name: "atari7800_init_160a", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 19, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 11168014748555143245
^19 = gv: (name: "atari7800_scene_set_palette", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 13, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 12448712353131340802
^20 = gv: (name: "atari7800_configure_video", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 15, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 12794849391082309999
^21 = gv: (name: "atari7800_scene_display_list", summaries: (variable: (module: ^0, flags: (linkage: internal, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), varFlags: (readonly: 1, writeonly: 1, constant: 0)))) ; guid = 13983316710606083779
^22 = gv: (name: "atari7800_scene_draw_text", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 201, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), refs: (^14, ^16, ^1, ^21)))) ; guid = 14379036656215668754
^23 = gv: (name: "atari7800_maria_plot_sprite_zone5", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 28, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 14761944858682005363
^24 = gv: (name: "atari7800_scene_end_frame", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 32, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), refs: (^12, ^14, ^17, ^16, ^21)))) ; guid = 15586239553480496635
^25 = gv: (name: "atari7800_maria_build_ntsc_single_zone", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 18, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0), calls: ((callee: ^7, tail: 1))))) ; guid = 15933352953334360056
^26 = gv: (name: "atari7800_scene_plotchars", summaries: (function: (module: ^0, flags: (linkage: external, visibility: default, notEligibleToImport: 1, live: 0, dsoLocal: 1, canAutoHide: 0, importType: definition, noRenameOnPromotion: 0), insts: 37, funcFlags: (readNone: 0, readOnly: 0, noRecurse: 1, returnDoesNotAlias: 0, noInline: 0, alwaysInline: 0, noUnwind: 1, mayThrow: 0, hasUnknownCall: 0, mustBeUnreachable: 0)))) ; guid = 16451751229686798378
^27 = flags: 8
^28 = blockcount: 0
