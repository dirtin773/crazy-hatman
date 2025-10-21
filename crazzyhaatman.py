import pygame
import sys
import math
import random

# Инициализация Pygame
pygame.init()

# Настройки экрана
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Crazy Hatman")

# Цвета
WHITE = (255, 255, 255)
RED = (255, 0, 0)
BLUE = (0, 0, 255)
GREEN = (0, 255, 0)
BLACK = (0, 0, 0)
GRAY = (150, 150, 150)
LIGHT_BLUE = (100, 100, 255)
YELLOW = (255, 255, 0)
PURPLE = (255, 0, 255)
ORANGE = (255, 165, 0)
DARK_RED = (139, 0, 0)
GOLD = (255, 215, 0)
CYAN = (0, 255, 255)
DARK_BLUE = (0, 0, 139)


# Класс кнопки
class Button:
    def __init__(self, x, y, width, height, text, color=LIGHT_BLUE, hover_color=(150, 150, 255)):
        self.rect = pygame.Rect(x, y, width, height)
        self.text = text
        self.color = color
        self.hover_color = hover_color
        self.current_color = color
        self.font = pygame.font.SysFont(None, 36)

    def draw(self, surface):
        pygame.draw.rect(surface, self.current_color, self.rect, border_radius=10)
        pygame.draw.rect(surface, WHITE, self.rect, 2, border_radius=10)

        text_surf = self.font.render(self.text, True, WHITE)
        text_rect = text_surf.get_rect(center=self.rect.center)
        surface.blit(text_surf, text_rect)

    def is_hovered(self, pos):
        return self.rect.collidepoint(pos)

    def update(self, pos):
        if self.is_hovered(pos):
            self.current_color = self.hover_color
        else:
            self.current_color = self.color


# Класс ножа
class Knife:
    def __init__(self, x, y, direction_angle):
        self.x = x
        self.y = y
        self.speed = 8
        self.radius = 5
        self.color = RED
        self.direction_angle = direction_angle
        self.distance_traveled = 0
        self.max_distance = 200

    def update(self):
        self.x += math.cos(self.direction_angle) * self.speed
        self.y += math.sin(self.direction_angle) * self.speed
        self.distance_traveled += self.speed

    def draw(self):
        angle = self.direction_angle
        tip_x = self.x + math.cos(angle) * 10
        tip_y = self.y + math.sin(angle) * 10

        blade_angle1 = angle + math.pi / 2
        blade_angle2 = angle - math.pi / 2

        point1_x = self.x + math.cos(blade_angle1) * 3
        point1_y = self.y + math.sin(blade_angle1) * 3
        point2_x = self.x + math.cos(blade_angle2) * 3
        point2_y = self.y + math.sin(blade_angle2) * 3

        points = [(tip_x, tip_y), (point1_x, point1_y), (point2_x, point2_y)]
        pygame.draw.polygon(screen, RED, points)

        handle_x = self.x - math.cos(angle) * 5
        handle_y = self.y - math.sin(angle) * 5
        pygame.draw.rect(screen, (139, 69, 19), (handle_x - 3, handle_y - 2, 6, 4))

    def is_off_screen(self):
        return (self.x < -20 or self.x > WIDTH + 20 or
                self.y < -20 or self.y > HEIGHT + 20 or
                self.distance_traveled >= self.max_distance)

    def collides_with(self, enemy):
        distance = math.sqrt((self.x - enemy.x) ** 2 + (self.y - enemy.y) ** 2)
        return distance < self.radius + enemy.radius


# Класс бонуса-шляпы
class HatBonus:
    def __init__(self, x, y, bonus_type="damage"):
        self.x = x
        self.y = y
        self.width = 30
        self.height = 25
        self.bonus_type = bonus_type
        self.color = GOLD if bonus_type == "damage" else RED
        self.speed = 2
        self.collected = False
        self.lifetime = 300

    def update(self):
        self.y += self.speed
        self.lifetime -= 1
        return self.lifetime <= 0 or self.y > HEIGHT

    def draw(self):
        pygame.draw.rect(screen, self.color, (self.x - self.width // 2, self.y, self.width, 10))
        pygame.draw.rect(screen, self.color, (self.x - self.width // 3, self.y - 15, self.width // 1.5, 15))

        sparkle_color = YELLOW if self.bonus_type == "damage" else WHITE
        pygame.draw.rect(screen, sparkle_color, (self.x - self.width // 2 + 5, self.y + 2, 5, 3))
        pygame.draw.rect(screen, sparkle_color, (self.x + self.width // 2 - 10, self.y + 2, 5, 3))

        if self.bonus_type == "knife":
            knife_points = [
                (self.x, self.y - 5),
                (self.x - 3, self.y + 2),
                (self.x + 3, self.y + 2)
            ]
            pygame.draw.polygon(screen, WHITE, knife_points)

    def collides_with_player(self, player):
        player_rect = pygame.Rect(player.x - player.radius, player.y - player.radius,
                                  player.radius * 2, player.radius * 2)
        bonus_rect = pygame.Rect(self.x - self.width // 2, self.y - 15, self.width, 25)
        return player_rect.colliderect(bonus_rect)


# Класс игрока
class Player:
    def __init__(self):
        self.x = WIDTH // 2
        self.y = HEIGHT - 50
        self.radius = 20
        self.speed = 5
        self.color = BLUE
        self.health = 10
        self.lives = 1
        self.invincible = False
        self.invincible_timer = 0
        self.damage_multiplier = 1
        self.bonus_timer = 0
        self.has_knife_bonus = False

    def draw(self):
        if self.has_knife_bonus:
            hat_color = RED
        elif self.damage_multiplier > 1:
            hat_color = GOLD
        else:
            hat_color = DARK_BLUE

        pygame.draw.rect(screen, hat_color, (self.x - 25, self.y - self.radius - 20, 50, 15))
        pygame.draw.rect(screen, hat_color, (self.x - 15, self.y - self.radius - 35, 30, 20))

        body_color = RED if self.invincible else BLUE
        pygame.draw.circle(screen, body_color, (self.x, self.y), self.radius)

        eye_color = WHITE
        pygame.draw.circle(screen, eye_color, (self.x - 8, self.y - 5), 5)
        pygame.draw.circle(screen, eye_color, (self.x + 8, self.y - 5), 5)

        mouse_x, mouse_y = pygame.mouse.get_pos()
        dx = mouse_x - self.x
        dy = mouse_y - self.y
        distance = max(math.sqrt(dx * dx + dy * dy), 1)

        pupil_offset = 2
        pupil_x1 = self.x - 8 + (dx / distance) * pupil_offset
        pupil_y1 = self.y - 5 + (dy / distance) * pupil_offset
        pupil_x2 = self.x + 8 + (dx / distance) * pupil_offset
        pupil_y2 = self.y - 5 + (dy / distance) * pupil_offset

        pygame.draw.circle(screen, BLACK, (int(pupil_x1), int(pupil_y1)), 2)
        pygame.draw.circle(screen, BLACK, (int(pupil_x2), int(pupil_y2)), 2)

        mouth_y = self.y + 5
        if self.health <= 3:
            pygame.draw.arc(screen, BLACK, (self.x - 8, mouth_y - 2, 16, 10), math.pi, 2 * math.pi, 2)
        elif self.invincible:
            pygame.draw.arc(screen, RED, (self.x - 8, mouth_y, 16, 8), 0, math.pi, 2)
        else:
            pygame.draw.line(screen, BLACK, (self.x - 6, mouth_y), (self.x + 6, mouth_y), 2)

        health_width = 40
        health_height = 6
        health_x = self.x - health_width // 2
        health_y = self.y - self.radius - 10

        pygame.draw.rect(screen, RED, (health_x, health_y, health_width, health_height))
        health_percent = self.health / 10.0
        pygame.draw.rect(screen, GREEN, (health_x, health_y, health_width * health_percent, health_height))

        health_text = pygame.font.SysFont(None, 16).render(f"{self.health}/10", True, WHITE)
        screen.blit(health_text, (self.x - health_text.get_width() // 2, health_y - 15))

    def move(self, keys):
        if keys[pygame.K_LEFT] and self.x - self.radius > 0:
            self.x -= self.speed
        if keys[pygame.K_RIGHT] and self.x + self.radius < WIDTH:
            self.x += self.speed
        if keys[pygame.K_UP] and self.y - self.radius > 0:
            self.y -= self.speed
        if keys[pygame.K_DOWN] and self.y + self.radius < HEIGHT:
            self.y += self.speed

    def take_damage(self, damage=1):
        if not self.invincible:
            self.health -= damage
            self.invincible = True
            self.invincible_timer = 90

            if self.health <= 0:
                self.lives -= 1
                self.health = 0
                self.damage_multiplier = 1
                self.bonus_timer = 0
                self.has_knife_bonus = False
                return True
        return False

    def update(self):
        if self.invincible:
            self.invincible_timer -= 1
            if self.invincible_timer <= 0:
                self.invincible = False

        if self.damage_multiplier > 1:
            self.bonus_timer -= 1
            if self.bonus_timer <= 0:
                self.damage_multiplier = 1

    def add_damage_bonus(self):
        self.damage_multiplier = 2
        self.bonus_timer = 600
        self.has_knife_bonus = False

    def add_knife_bonus(self):
        self.has_knife_bonus = True
        self.damage_multiplier = 1
        self.bonus_timer = 0

    def is_alive(self):
        return self.lives > 0 and self.health > 0

    def create_knives(self, x, y):
        knives = []
        angles = [0, 72, 144, 216, 288]
        for angle in angles:
            angle_rad = math.radians(angle)
            knives.append(Knife(x, y, angle_rad))
        return knives


# Класс пули
class Bullet:
    def __init__(self, x, y, target_x, target_y, level=1, damage_multiplier=1):
        self.x = x
        self.y = y
        self.radius = 8
        self.speed = 7 + level
        self.color = GREEN if level == 1 else YELLOW if level == 2 else PURPLE
        self.damage = level * damage_multiplier

        dx = target_x - x
        dy = target_y - y
        distance = max(math.sqrt(dx * dx + dy * dy), 0.1)

        self.dx = dx / distance * self.speed
        self.dy = dy / distance * self.speed
        self.level = level

    def update(self):
        self.x += self.dx
        self.y += self.dy

    def draw(self):
        pygame.draw.circle(screen, self.color, (int(self.x), int(self.y)), self.radius)
        if self.level > 1:
            pygame.draw.circle(screen, WHITE, (int(self.x), int(self.y)), self.radius - 3)

    def is_off_screen(self):
        return (self.x < -self.radius or self.x > WIDTH + self.radius or
                self.y < -self.radius or self.y > HEIGHT + self.radius)


# Класс врага
class Enemy:
    def __init__(self, level=1, is_elite=False):
        self.level = level
        self.is_elite = is_elite

        if is_elite:
            self.radius = 30
            self.speed = 1.5
            self.color = GOLD
            self.health = 5
            self.damage = 2
            self.score_value = 50
        elif level == 1:
            self.radius = 15
            self.speed = 2
            self.color = RED
            self.health = 3
            self.damage = 1
            self.score_value = 10
        elif level == 2:
            self.radius = 20
            self.speed = 2.5
            self.color = ORANGE
            self.health = 4
            self.damage = 1
            self.score_value = 20
        else:
            self.radius = 25
            self.speed = 3
            self.color = DARK_RED
            self.health = 5
            self.damage = 2
            self.score_value = 30

        self.x = random.randint(self.radius, WIDTH - self.radius)
        self.y = -self.radius

        self.dx = random.uniform(-1, 1)
        self.dy = random.uniform(0.5, 1.5)

        length = math.sqrt(self.dx * self.dx + self.dy * self.dy)
        self.dx = self.dx / length * self.speed
        self.dy = self.dy / length * self.speed

        self.attack_cooldown = 0

    def update(self):
        self.x += self.dx
        self.y += self.dy

        if self.attack_cooldown > 0:
            self.attack_cooldown -= 1

    def draw(self):
        pygame.draw.circle(screen, self.color, (int(self.x), int(self.y)), self.radius)

        if self.is_elite:
            pygame.draw.circle(screen, YELLOW, (int(self.x), int(self.y)), self.radius - 5, 2)
            crown_points = [
                (self.x - 15, self.y - self.radius + 5),
                (self.x - 10, self.y - self.radius - 5),
                (self.x - 5, self.y - self.radius + 2),
                (self.x, self.y - self.radius - 8),
                (self.x + 5, self.y - self.radius + 2),
                (self.x + 10, self.y - self.radius - 5),
                (self.x + 15, self.y - self.radius + 5)
            ]
            pygame.draw.polygon(screen, YELLOW, crown_points)

        eye_offset = self.radius * 0.4
        eye_color = CYAN if self.is_elite else BLACK
        pygame.draw.circle(screen, eye_color, (int(self.x - eye_offset), int(self.y - eye_offset)), 4)
        pygame.draw.circle(screen, eye_color, (int(self.x + eye_offset), int(self.y - eye_offset)), 4)

        mouth_y = self.y + eye_offset
        if self.is_elite:
            pygame.draw.arc(screen, RED, (self.x - eye_offset, mouth_y - 3, eye_offset * 2, 8), math.pi, 2 * math.pi, 3)
        else:
            pygame.draw.arc(screen, BLACK, (self.x - eye_offset, mouth_y - 2, eye_offset * 2, 5), math.pi, 2 * math.pi,
                            2)

        health_text = pygame.font.SysFont(None, 20).render(str(self.health), True, WHITE)
        screen.blit(health_text, (self.x - 5, self.y - 7))

    def is_off_screen(self):
        return (self.y > HEIGHT + self.radius)

    def take_damage(self, damage=1):
        self.health -= damage
        return self.health <= 0

    def collides_with(self, bullet):
        distance = math.sqrt((self.x - bullet.x) ** 2 + (self.y - bullet.y) ** 2)
        return distance < self.radius + bullet.radius

    def collides_with_player(self, player):
        if player.invincible:
            return False

        distance = math.sqrt((self.x - player.x) ** 2 + (self.y - player.y) ** 2)
        collided = distance < self.radius + player.radius

        if collided and self.attack_cooldown == 0:
            self.attack_cooldown = 30
            return True
        return False


# Класс игры
class Game:
    def __init__(self):
        self.state = "menu"
        self.level = 1
        self.max_level = 3
        self.score = 0
        self.enemies_to_defeat = 10
        self.enemies_defeated = 0
        self.player = Player()
        self.bullets = []
        self.enemies = []
        self.hat_bonuses = []
        self.knives = []
        self.enemy_spawn_timer = 0
        self.level_complete_timer = 0
        self.bonus_spawn_timer = 0

        button_width, button_height = 200, 50
        center_x = WIDTH // 2 - button_width // 2

        self.new_game_button = Button(center_x, 250, button_width, button_height, "Новая игра")
        self.continue_button = Button(center_x, 320, button_width, button_height, "Продолжить")
        self.quit_button = Button(center_x, 390, button_width, button_height, "Выход")

        self.font = pygame.font.SysFont(None, 36)
        self.title_font = pygame.font.SysFont(None, 72)
        self.small_font = pygame.font.SysFont(None, 24)

    def spawn_enemy(self):
        spawn_chance = random.random()

        if random.random() < 0.03:
            self.enemies.append(Enemy(level=3, is_elite=True))
            return

        if self.level == 1:
            enemy_level = 1
        elif self.level == 2:
            enemy_level = 1 if spawn_chance < 0.7 else 2
        else:
            enemy_level = 1 if spawn_chance < 0.5 else 2 if spawn_chance < 0.8 else 3

        self.enemies.append(Enemy(enemy_level))

    def spawn_hat_bonus(self, x, y):
        bonus_type = "damage" if random.random() < 0.5 else "knife"
        self.hat_bonuses.append(HatBonus(x, y, bonus_type))

    def handle_events(self):
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False

            elif event.type == pygame.MOUSEBUTTONDOWN:
                mouse_pos = pygame.mouse.get_pos()

                if self.state == "menu":
                    if self.new_game_button.is_hovered(mouse_pos):
                        self.start_new_game()
                    elif self.continue_button.is_hovered(mouse_pos):
                        if self.state == "menu" and self.score > 0:
                            self.state = "playing"
                    elif self.quit_button.is_hovered(mouse_pos):
                        return False

                elif self.state == "playing" and event.button == 1:
                    mouse_x, mouse_y = pygame.mouse.get_pos()

                    if self.player.has_knife_bonus:
                        self.knives.extend(self.player.create_knives(self.player.x, self.player.y))
                        self.player.has_knife_bonus = False
                    else:
                        self.bullets.append(Bullet(self.player.x, self.player.y, mouse_x, mouse_y,
                                                   self.level, self.player.damage_multiplier))

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    if self.state == "playing":
                        self.state = "menu"
                    elif self.state == "menu":
                        return False

                elif event.key == pygame.K_RETURN:
                    if self.state == "level_complete" and self.level_complete_timer > 60:
                        self.start_next_level()

        return True

    def start_new_game(self):
        self.state = "playing"
        self.level = 1
        self.score = 0
        self.enemies_defeated = 0
        self.player = Player()
        self.bullets = []
        self.enemies = []
        self.hat_bonuses = []
        self.knives = []

    def start_next_level(self):
        if self.level < self.max_level:
            self.level += 1
            self.enemies_defeated = 0
            self.bullets = []
            self.enemies = []
            self.hat_bonuses = []
            self.knives = []
            self.player.invincible = True
            self.player.invincible_timer = 120
            self.state = "playing"
        else:
            self.state = "menu"

    def update(self):
        if self.state == "playing":
            keys = pygame.key.get_pressed()
            self.player.move(keys)
            self.player.update()

            if not self.player.is_alive():
                self.state = "game_over"
                return

            self.enemy_spawn_timer += 1
            spawn_rate = max(70 - self.level * 10, 30)
            if self.enemy_spawn_timer >= spawn_rate:
                self.spawn_enemy()
                self.enemy_spawn_timer = 0

            self.bonus_spawn_timer += 1
            if self.bonus_spawn_timer >= 450 and random.random() < 0.15:
                x = random.randint(50, WIDTH - 50)
                self.spawn_hat_bonus(x, -50)
                self.bonus_spawn_timer = 0

            for bullet in self.bullets[:]:
                bullet.update()
                if bullet.is_off_screen():
                    self.bullets.remove(bullet)

            for knife in self.knives[:]:
                knife.update()
                if knife.is_off_screen():
                    self.knives.remove(knife)
                    continue

                for enemy in self.enemies[:]:
                    if knife.collides_with(enemy):
                        if enemy.take_damage(2):
                            self.enemies.remove(enemy)
                            self.score += enemy.score_value
                            self.enemies_defeated += 1

                            if random.random() < 0.15:
                                self.spawn_hat_bonus(enemy.x, enemy.y)

                        if knife in self.knives:
                            self.knives.remove(knife)
                        break

            for enemy in self.enemies[:]:
                enemy.update()

                if enemy.is_off_screen():
                    self.enemies.remove(enemy)
                    continue

                if enemy.collides_with_player(self.player):
                    self.player.take_damage(enemy.damage)

                for bullet in self.bullets[:]:
                    if enemy.collides_with(bullet):
                        if enemy.take_damage(bullet.damage):
                            self.enemies.remove(enemy)
                            self.score += enemy.score_value
                            self.enemies_defeated += 1

                            if self.player.has_knife_bonus:
                                self.knives.extend(self.player.create_knives(enemy.x, enemy.y))
                                self.player.has_knife_bonus = False
                            else:
                                if random.random() < 0.10:
                                    self.spawn_hat_bonus(enemy.x, enemy.y)

                        if bullet in self.bullets:
                            self.bullets.remove(bullet)
                        break

            for bonus in self.hat_bonuses[:]:
                if bonus.update():
                    self.hat_bonuses.remove(bonus)
                elif bonus.collides_with_player(self.player):
                    if bonus.bonus_type == "damage":
                        self.player.add_damage_bonus()
                    else:
                        self.player.add_knife_bonus()
                    self.hat_bonuses.remove(bonus)

            if self.enemies_defeated >= self.enemies_to_defeat:
                self.state = "level_complete"
                self.level_complete_timer = 0

            mouse_pos = pygame.mouse.get_pos()
            self.new_game_button.update(mouse_pos)
            self.continue_button.update(mouse_pos)
            self.quit_button.update(mouse_pos)

        elif self.state == "level_complete":
            self.level_complete_timer += 1

        elif self.state == "game_over":
            pass

    def draw(self):
        screen.fill(BLACK)

        if self.state == "menu":
            self.draw_menu()
        elif self.state == "playing":
            self.draw_game()
        elif self.state == "level_complete":
            self.draw_level_complete()
        elif self.state == "game_over":
            self.draw_game_over()

        pygame.display.flip()

    def draw_menu(self):
        title = self.title_font.render("CRAZY HATMAN", True, LIGHT_BLUE)
        screen.blit(title, (WIDTH // 2 - title.get_width() // 2, 100))

        subtitle = self.font.render("Одна жизнь, десять здоровья - выживи любой ценой!", True, WHITE)
        screen.blit(subtitle, (WIDTH // 2 - subtitle.get_width() // 2, 170))

        hint_text = self.small_font.render("Собирайте шляпы для особых способностей!", True, YELLOW)
        screen.blit(hint_text, (WIDTH // 2 - hint_text.get_width() // 2, 200))

        self.new_game_button.draw(screen)
        self.continue_button.draw(screen)
        self.quit_button.draw(screen)

        if self.score > 0:
            score_text = self.font.render(f"Лучший счет: {self.score}", True, WHITE)
            screen.blit(score_text, (WIDTH // 2 - score_text.get_width() // 2, 460))

    def draw_game(self):
        self.player.draw()
        for bullet in self.bullets:
            bullet.draw()
        for enemy in self.enemies:
            enemy.draw()
        for bonus in self.hat_bonuses:
            bonus.draw()
        for knife in self.knives:
            knife.draw()

        score_text = self.font.render(f"Счет: {self.score}", True, WHITE)
        level_text = self.font.render(f"Уровень: {self.level}", True, WHITE)
        progress_text = self.font.render(f"Врагов: {self.enemies_defeated}/{self.enemies_to_defeat}", True, WHITE)
        lives_text = self.font.render(f"Жизнь: {self.player.lives}/1", True, GREEN if self.player.lives > 0 else RED)

        screen.blit(score_text, (10, 10))
        screen.blit(level_text, (10, 50))
        screen.blit(progress_text, (10, 90))
        screen.blit(lives_text, (WIDTH - 150, 10))

        bonus_y = 130
        if self.player.damage_multiplier > 1:
            damage_bonus = self.small_font.render(f"УРОН x2 ({self.player.bonus_timer // 60}сек)", True, YELLOW)
            screen.blit(damage_bonus, (10, bonus_y))
            bonus_y += 25

        if self.player.has_knife_bonus:
            knife_bonus = self.small_font.render("НОЖИ ГОТОВЫ! Кликните для выстрела", True, RED)
            screen.blit(knife_bonus, (10, bonus_y))

        if self.player.health <= 3:
            health_warning = self.font.render(f"ОСТОРОЖНО! Здоровье: {self.player.health}/10", True, RED)
            screen.blit(health_warning, (WIDTH // 2 - health_warning.get_width() // 2, HEIGHT - 120))

        if self.player.invincible:
            inv_text = self.small_font.render("НЕУЯЗВИМОСТЬ!", True, YELLOW)
            screen.blit(inv_text, (WIDTH // 2 - inv_text.get_width() // 2, HEIGHT - 30))

    def draw_level_complete(self):
        self.draw_game()

        overlay = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 150))
        screen.blit(overlay, (0, 0))

        if self.level < self.max_level:
            message = self.title_font.render(f"УРОВЕНЬ {self.level} ПРОЙДЕН!", True, YELLOW)
            next_level = self.font.render("Нажмите ENTER для перехода к следующему уровню", True, WHITE)
        else:
            message = self.title_font.render("ВЫ ПРОШЛИ ИГРУ!", True, YELLOW)
            next_level = self.font.render("Нажмите ENTER для возврата в меню", True, WHITE)

        screen.blit(message, (WIDTH // 2 - message.get_width() // 2, HEIGHT // 2 - 50))
        screen.blit(next_level, (WIDTH // 2 - next_level.get_width() // 2, HEIGHT // 2 + 30))

        health_text = self.font.render(f"Осталось здоровья: {self.player.health}/10", True,
                                       GREEN if self.player.health > 5 else YELLOW if self.player.health > 2 else RED)
        screen.blit(health_text, (WIDTH // 2 - health_text.get_width() // 2, HEIGHT // 2 + 80))

        stats_text = self.font.render(f"Итоговый счет: {self.score}", True, WHITE)
        screen.blit(stats_text, (WIDTH // 2 - stats_text.get_width() // 2, HEIGHT // 2 + 120))

    def draw_game_over(self):
        self.draw_game()

        overlay = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 200))
        screen.blit(overlay, (0, 0))

        game_over_text = self.title_font.render("GAME OVER", True, RED)
        score_text = self.font.render(f"Ваш счет: {self.score}", True, WHITE)
        level_text = self.font.render(f"Достигнутый уровень: {self.level}", True, WHITE)
        restart_text = self.font.render("Нажмите ESC для возврата в меню", True, WHITE)

        screen.blit(game_over_text, (WIDTH // 2 - game_over_text.get_width() // 2, HEIGHT // 2 - 80))
        screen.blit(score_text, (WIDTH // 2 - score_text.get_width() // 2, HEIGHT // 2 - 20))
        screen.blit(level_text, (WIDTH // 2 - level_text.get_width() // 2, HEIGHT // 2 + 20))
        screen.blit(restart_text, (WIDTH // 2 - restart_text.get_width() // 2, HEIGHT // 2 + 60))


# Главная функция
def main():
    game = Game()
    clock = pygame.time.Clock()

    running = True
    while running:
        running = game.handle_events()
        game.update()
        game.draw()
        clock.tick(60)

    pygame.quit()
    sys.exit()


if __name__ == "__main__":
    main()