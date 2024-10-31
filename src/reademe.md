ffmpeg -r 30 -i frame_%03d.png -c:v libx264 -vf fps=30 -pix_fmt yuv420p output.mp4
