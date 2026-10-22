using FlagGuessingGameAPI.Controllers.Commands;
using FlagGuessingGameAPI.Models;
using FlagGuessingGameAPI.Repositories;
using Microsoft.AspNetCore.Mvc;

namespace FlagGuessingGameAPI.Controllers
{
    [ApiController]
    [Route("api/[controller]")]
    public class PlayerController : ControllerBase
    {
        private readonly IPlayerRepository _playerRepository;

        public PlayerController(IPlayerRepository playerRepository)
        {
            _playerRepository = playerRepository;
        }

        [HttpPost("Create")]
        public async Task<int> CreatePlayer([FromBody] CreatePlayerCommand command, CancellationToken cancellationToken)
        {
            var player = await _playerRepository.GetPlayerByNameAsync(command.Name, cancellationToken);

            if (player is null)
            {
                player = new Player
                {
                    Name = command.Name,
                    HighScore = 0
                };

                await _playerRepository.AddPlayerAsync(player, cancellationToken);
            }

            return player.Id;
        }

        [HttpGet("{playerId}")]
        public async Task<IActionResult> GetPlayer(int playerId, CancellationToken cancellationToken)
        {
            var player = await _playerRepository.GetPlayerByIdAsync(playerId, cancellationToken);

            if (player is null)
            {
                return NotFound();
            }

            return Ok(player);
        }

        [HttpGet]
        public async Task<IActionResult> GetLeaderboard(CancellationToken cancellationToken)
        {
            return Ok(await _playerRepository.GetAllPlayersAsync(cancellationToken));
        }

        [HttpPut("{playerId}/Score")]
        public async Task<IActionResult> UpdatePlayerScore(int playerId, [FromBody] UpdateScoreCommand command, CancellationToken cancellationToken)
        {
            var player = await _playerRepository.GetPlayerByIdAsync(playerId, cancellationToken);

            if (player is null)
            {
                return NotFound();
            }

            if (command.Score > player.HighScore)
            {
                player.HighScore = command.Score;
            }

            await _playerRepository.SaveChangesAsync(cancellationToken);

            return Ok(player);
        }
    }
}
