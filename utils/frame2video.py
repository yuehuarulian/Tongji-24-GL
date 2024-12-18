import cv2
import os

# 设置输入和输出路径
image_folder = r'C:\Users\13294\Desktop\Denoise\scene2\denoised_images'
output_video = 'output_video.mp4'

# 视频参数
fps = 30  # 每秒帧数
frame_size = (1080 * 2, 720 * 2, )  # 设置视频分辨率 (宽, 高)，需与图片大小一致

# 获取图片文件列表并排序
images = [img for img in os.listdir(image_folder) if img.endswith(".png")]
images.sort()  # 确保按顺序读取帧

# 创建视频写入器
fourcc = cv2.VideoWriter_fourcc(*'mp4v')  # 指定视频编码
video_writer = cv2.VideoWriter(output_video, fourcc, fps, frame_size)

# 逐帧写入视频
for image in images:
    img_path = os.path.join(image_folder, image)
    frame = cv2.imread(img_path)
    
    # 检查图片是否读取成功
    if frame is None:
        print(f"Could not read image {img_path}")
        continue

    # 调整图片大小以匹配视频分辨率
    frame = cv2.resize(frame, frame_size)
    video_writer.write(frame)  # 写入视频帧
    print(f"Added {img_path} to video.")

# 释放视频写入器资源
video_writer.release()
print(f"Video saved as {output_video}")
