using ArduinoLandeQuizAPI.Controllers.Commands;
using ArduinoLandeQuizAPI.Models;
using ArduinoLandeQuizAPI.Repositories;
using Microsoft.AspNetCore.Mvc;

namespace ArduinoLandeQuizAPI.Controllers
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
            return await _playerRepository.AddPlayerAsync(new Player { Name = command.Name }, cancellationToken);
        }

        [HttpGet("{playerId}")]
        public async Task<IActionResult> GetPlayer(int playerId, CancellationToken cancellationToken)
        {
            var player = await _playerRepository.GetPlayerAsync(playerId, cancellationToken);

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
            var player = await _playerRepository.GetPlayerAsync(playerId, cancellationToken);

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
