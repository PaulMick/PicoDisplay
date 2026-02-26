import pygame

# This popup assists in the creation of variable width binary bitmaps fonts, with varying widths
# 4 bytes (32 bits) will be used to store the representation of each of the 128 standard ASCII characters in order
# The first 25 (on the little endian side) represent the up to 5x5 pixel bitmap, and the last 7 specify the width of the character
# e.g. 0b00001011111111111111111111111111 would store a 5x5 square that is completely filled in
#        ^width^^--------bitmap---------^

pygame.init()
screen = pygame.display.set_mode((600, 500))
pygame.display.set_caption("Create 3x5 Font")
button_font = pygame.font.SysFont("Arial", 20)
next_text = button_font.render("Next", True, (255, 255, 255))
back_text = button_font.render("Back", True, (255, 255, 255))

grid = [
    [0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0]
]

grids = []

# Character widths are automatically calculated, but can be overriden by entries in this override dict (for example, and entry of 32: 3 would ensure that the [SPACE] character is 3 pixels wide)
# It's generally best to let widths be determined automatically, but the overrides are useful for ensuring whitespace characters such as [SPACE] have enough width reserved for them
spacing_overrides = {
    9: 6, # [HORIZONTAL TAB]
    32: 3 # [SPACE]
}

font_name = "font_5x5"

# Pack each character bitmap into 4 bytes and write it to the file
def print_font(grids: list[list[list[int]]], fname: str) -> None:
    print(f"static uint32_t {font_name}[128] = {'{'}")
    for a in range(len(grids)):
        n = 0x00000000
        for bit in range(25):
            i = int(bit / 5)
            j = bit - i * 5
            n |= grids[a][i][j] << bit
        # Automatically determine width
        width = 0
        if n == 0:
            width = 0
        elif n < 0x20:
            width = 1
        elif n < 0x400:
            width = 2
        elif n < 0x8000:
            width = 3
        elif n < 0x100000:
            width = 4
        else:
            width = 5
        # If there is an override for this character, do it
        if a in spacing_overrides:
            width = spacing_overrides[a]
        n |= width << 25
        b = n.to_bytes(4, "big")
        print(f"    {n:#0{10}x},")
    print("};")

running = True
while running:
    # When the last character is confirmed, pack and write to file
    if len(grids) >= 128:
        print_font(grids, "3x5.font")
        exit(0)
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.MOUSEBUTTONDOWN:
            # Handle clicks
            click_x, click_y = event.pos
            i, j = int(click_x  / 100), int(click_y / 100)
            if i <= 4:
                # Click on the pixel grid
                if grid[i][j] == 0:
                    grid[i][j] = 1
                else:
                    grid[i][j] = 0
            elif i == 5:
                if j == 3:
                    # Click on the 'Next' button
                    grids.append(grid)
                    grid = [
                        [0, 0, 0, 0, 0],
                        [0, 0, 0, 0, 0],
                        [0, 0, 0, 0, 0],
                        [0, 0, 0, 0, 0],
                        [0, 0, 0, 0, 0]
                    ]
                elif j == 4:
                    # Click on the 'Back' button
                    if len(grids) > 0:
                        grid = grids.pop()

    # Draw grid
    pygame.draw.line(screen, (255, 255, 255), (0, 0), (500, 0), width = 2)
    pygame.draw.line(screen, (255, 255, 255), (0, 100), (500, 100), width = 2)
    pygame.draw.line(screen, (255, 255, 255), (0, 200), (500, 200), width = 2)
    pygame.draw.line(screen, (255, 255, 255), (0, 300), (600, 300), width = 2)
    pygame.draw.line(screen, (255, 255, 255), (0, 400), (600, 400), width = 2)
    pygame.draw.line(screen, (255, 255, 255), (0, 0), (0, 500), width = 2)
    pygame.draw.line(screen, (100, 100, 255), (100, 0), (100, 500), width = 2)
    pygame.draw.line(screen, (100, 255, 255), (200, 0), (200, 500), width = 2)
    pygame.draw.line(screen, (100, 255, 100), (300, 0), (300, 500), width = 2)
    pygame.draw.line(screen, (255, 255, 100), (400, 0), (400, 500), width = 2)
    pygame.draw.line(screen, (255, 100, 100), (500, 0), (500, 500), width = 2)
    # Draw pixels
    for x in range(5):
        for y in range(5):
            pygame.draw.rect(screen, (grid[x][y] * 255, grid[x][y] * 255, grid[x][y] * 255), (x * 100 + 2, y * 100 + 2, 98, 98))
    # Draw side buttons/counter
    pygame.draw.rect(screen, (100, 0, 0), (502, 402, 98, 98))
    pygame.draw.rect(screen, (0, 100, 0), (502, 302, 98, 98))
    pygame.draw.rect(screen, (0, 0, 0), (502, 2, 98, 98))
    counter_text = button_font.render(f"{len(grids)}", True, (255, 255, 255))
    screen.blit(counter_text, (533, 43))
    screen.blit(next_text, (533, 343))
    screen.blit(back_text, (533, 443))
    
    
    pygame.display.flip()

pygame.quit()

