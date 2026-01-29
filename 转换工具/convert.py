import sys
import os
from PIL import Image

# ---------------------------------------------------------
# 鸿雪 3.6寸 E系列 7色屏硬件颜色定义
# ---------------------------------------------------------
# 格式: (R, G, B): 硬件指令索引
PALETTE_MAP = {
    (0, 0, 0):       0x0, # Black
    (255, 255, 255): 0x1, # White
    (255, 255, 0):   0x2, # Yellow
    (255, 0, 0):     0x3, # Red
    (0, 0, 255):     0x5, # Blue
    (0, 255, 0):     0x6, # Green
}
PALETTE_COLORS = list(PALETTE_MAP.keys())

def get_closest_color(pixel):
    """寻找颜色空间中距离最近的颜色"""
    min_dist = float('inf')
    closest_rgb = (255, 255, 255)
    r, g, b = pixel
    for pr, pg, pb in PALETTE_COLORS:
        dist = (r - pr)**2 + (g - pg)**2 + (b - pb)**2
        if dist < min_dist:
            min_dist = dist
            closest_rgb = (pr, pg, pb)
    return closest_rgb

def add_error(pixel, error, factor):
    r = pixel[0] + error[0] * factor
    g = pixel[1] + error[1] * factor
    b = pixel[2] + error[2] * factor
    return (min(255, max(0, int(r))), min(255, max(0, int(g))), min(255, max(0, int(b))))

def process_image(img_path, output_bin_path):
    print(f"Processing: {img_path}")
    
    # 1. 打开图片
    try:
        img = Image.open(img_path)
    except IOError:
        print(f"Error: Cannot open {img_path}")
        return

    # 2. 处理透明背景 (PNG)
    if img.mode in ('RGBA', 'LA') or (img.mode == 'P' and 'transparency' in img.info):
        print("  Detected Transparency. Converting background to WHITE...")
        background = Image.new("RGB", img.size, (255, 255, 255))
        if img.mode == 'P':
            img = img.convert('RGBA')
        background.paste(img, mask=img.split()[3])
        img = background
    else:
        img = img.convert("RGB")

    # 3. 强制缩放到 400x600
    print("  Resizing to 400x600...")
    img = img.resize((400, 600), Image.Resampling.LANCZOS)

    print("  Rotating 180 degrees...")
    img = img.rotate(180)
    
    width, height = img.size
    pixels = img.load()
    
    # 4. Floyd-Steinberg 抖动算法
    # 注意：这里直接修改了 img 对象内部的像素，所以处理完后 img 就是预览图
    print("  Applying Dithering...")
    for y in range(height):
        for x in range(width):
            old_pixel = pixels[x, y]
            new_pixel = get_closest_color(old_pixel)
            pixels[x, y] = new_pixel # 更新像素为7色之一
            
            error = (old_pixel[0]-new_pixel[0], old_pixel[1]-new_pixel[1], old_pixel[2]-new_pixel[2])
            
            # 误差扩散
            if x + 1 < width:
                pixels[x + 1, y] = add_error(pixels[x + 1, y], error, 7 / 16)
            if x - 1 >= 0 and y + 1 < height:
                pixels[x - 1, y + 1] = add_error(pixels[x - 1, y + 1], error, 3 / 16)
            if y + 1 < height:
                pixels[x, y + 1] = add_error(pixels[x, y + 1], error, 5 / 16)
            if x + 1 < width and y + 1 < height:
                pixels[x + 1, y + 1] = add_error(pixels[x + 1, y + 1], error, 1 / 16)

    # 5. 保存预览图 (PNG)
    # 自动生成预览文件名：比如 pass.bin -> pass_preview.png
    filename_root, _ = os.path.splitext(output_bin_path)
    preview_path = filename_root + "_preview.png"
    
    print(f"  Saving Preview Image to: {preview_path}")
    img.save(preview_path)

    # 6. 打包二进制数据 (.bin)
    print("  Packing binary data...")
    bin_data = bytearray()
    for y in range(height):
        for x in range(0, width, 2):
            # 此时 pixels 已经是处理过的 7色，直接查表即可
            c1 = PALETTE_MAP.get(pixels[x, y], 1)
            c2 = PALETTE_MAP.get(pixels[x+1, y], 1)
            bin_data.append((c1 << 4) | (c2 & 0x0F))
            
    with open(output_bin_path, "wb") as f:
        f.write(bin_data)
        
    print(f"Success! BIN Saved to {output_bin_path} ({len(bin_data)} bytes).")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python img2bin_preview.py <input_image> <output.bin>")
        print("Example: python img2bin_preview.py doctor.png pass.bin")
    else:
        process_image(sys.argv[1], sys.argv[2])