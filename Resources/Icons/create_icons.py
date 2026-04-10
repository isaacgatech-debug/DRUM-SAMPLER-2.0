from PIL import Image, ImageDraw

# Create home icon (house)
img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)
# Roof
draw.polygon([(10, 32), (32, 10), (54, 32)], fill='black')
# House body
draw.rectangle([16, 32, 48, 54], fill='black')
# Door
draw.rectangle([26, 40, 38, 54], fill='white')
img.save('home.png')

# Create mixer icon (sliders)
img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)
# Three vertical lines
draw.rectangle([15, 10, 18, 54], fill='black')
draw.rectangle([30, 10, 33, 54], fill='black')
draw.rectangle([45, 10, 48, 54], fill='black')
# Slider knobs
draw.ellipse([12, 18, 21, 27], fill='black')
draw.ellipse([27, 30, 36, 39], fill='black')
draw.ellipse([42, 42, 51, 51], fill='black')
img.save('mixer.png')

# Create drum kit icon
img = Image.new('RGBA', (64, 64), (0, 0, 0, 0))
draw = ImageDraw.Draw(img)
# Bass drum
draw.ellipse([20, 35, 44, 50], fill='black')
# Snare
draw.ellipse([10, 28, 26, 38], fill='black')
# Tom
draw.ellipse([38, 20, 52, 30], fill='black')
# Cymbal
draw.ellipse([8, 12, 20, 18], fill='black')
draw.line([(14, 18), (14, 28)], fill='black', width=2)
img.save('kit.png')

print("Icons created successfully")
